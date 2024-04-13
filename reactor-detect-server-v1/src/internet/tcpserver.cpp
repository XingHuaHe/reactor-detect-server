#include "tcpserver.h"



TcpServer::TcpServer(const uint16_t port, int threadnum):
	_threadnum(threadnum), _mainloop(new EventLoop(true, 30, 90)), _acceptor(_mainloop.get(), port), _threadpool(_threadnum, "IO")
{
	//_mainloop = new EventLoop();
	_mainloop->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));

	//_acceptor = new Acceptor(_mainloop, port);
	_acceptor.setnewconnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1));

	// 创建线程池
	//_threadpool = new ThreadPool(_threadnum, "IO");

	// 创建从事件"循环"
	for (int i = 0; i < _threadnum; i++)
	{
		_subloops.emplace_back(new EventLoop(false, 30, 90));
		//_subloops.push_back(new EventLoop());
		_subloops[i]->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1)); // 设置超时
		_subloops[i]->settimercallback(std::bind(&TcpServer::removeconn, this, std::placeholders::_1));	// 设置清理空闲TCP连接的回调函数
		// 如何运行从事件？将 run 函数作为任务扔给线程池
		_threadpool.addtask(std::bind(&EventLoop::run, _subloops[i].get()));	// 在线程池中运行从事件循环
	}
}

TcpServer::~TcpServer()
{
	//delete _acceptor;
	//delete _mainloop;

	//for (auto& it : _connsmap)	// 
	//{
	//	delete it.second;
	//}

	//for (auto& it : _subloops)	// 释放从事件
	//{
	//	delete it;
	//}

	//delete _threadpool;	//释放线程池
}

/*	运行事件循环	*/
void TcpServer::start()
{
	_mainloop->run();
}

/*	停止时间循环	*/
void TcpServer::stop()
{
	// 停止主事件循环
	_mainloop->stop();
	printf("mainloop stopped!\n");
	// 停止从事件循环
	for (int i = 0; i < _threadnum; i++)
	{
		_subloops[i]->stop();
	}
	printf("subloops stopped!\n");
	// 停止 IO 线程
	_threadpool.stop();
	printf("threadpool stopped!\n");
}

/*	处理新客户端连接请求	*/
void TcpServer::newconnection(std::unique_ptr<Socket> clientsock)
{
	/*	Connection 的生命周期：客户端连接，创建对象；客户端断开，销毁对象
		TcpServer 能够知道什么时候客户端连接，什么时候断开，故此，
		Connection 应该属于 TcpServer	*/
	spConnection conn(new Connection(_subloops[clientsock->fd() % _threadnum].get(), std::move(clientsock)));
	//Connection* conn = new Connection(_subloops[clientsock->fd()%_threadnum], clientsock);	// 采用取余随机选择从事件循环
	//Connection* conn = new Connection(_mainloop, clientsock);	// 指定仅在mainloop事件循环种处理
	conn->setclosecallback(std::bind(&TcpServer::closeconnection, this, std::placeholders::_1));
	conn->seterrorcallback(std::bind(&TcpServer::errorconnection, this, std::placeholders::_1));
	conn->setonmessagecallback(std::bind(&TcpServer::onmessage, this, std::placeholders::_1, std::placeholders::_2));
	conn->setsendcompletecallback(std::bind(&TcpServer::sendcomplete, this, std::placeholders::_1));

	{
		std::lock_guard<std::mutex> gd(_mutex);
		_connsmap[conn->fd()] = conn;	// 把 conn 存放在 TcpServer 的 map容器中
	}
	_subloops[conn->fd() % _threadnum]->newconnection(conn);	// 把conn存放到EventLoop的map容器中

	if (_newconnectioncb)
	{
		_newconnectioncb(conn);
	}
}

/*	关闭客户端的连接，在Connection类中回调此函数	*/
void TcpServer::closeconnection(spConnection conn)
{
	if (_closeconnectioncb)
	{
		_closeconnectioncb(conn);
	}

	{
		std::lock_guard<std::mutex> gd(_mutex);
		_connsmap.erase(conn->fd());
	}
	/*
	如果工作线程还未处理完，客户端关闭导致 TcpServer::closeconnection 被调用，
	那么会造成 conn 野指针出现，导致程序崩溃
	解决办法： Connection 采用智能指针
	*/
	//delete conn;
}

/*	客户端的连接错误，在Connection类中回调此函数	*/
void TcpServer::errorconnection(spConnection conn)
{
	if (_errorconnectioncb)
	{
		_errorconnectioncb(conn);
	}

	{
		std::lock_guard<std::mutex> gd(_mutex);
		_connsmap.erase(conn->fd());
	}
	//delete conn;
}

/*	处理客户端的请求报文，在Connection类中回调此函数	*/
void TcpServer::onmessage(spConnection conn, std::string& message)
{
	if (_onmessagecb)
	{
		_onmessagecb(conn, message);
	}
}

/*	数据发送完成后，在Connection类中回调此函数			*/
void TcpServer::sendcomplete(spConnection conn)
{
	if (_sendcompletecb)
	{
		_sendcompletecb(conn);
	}
}

/*	epoll_wait()超时，在EventLoop类中回调此函数			*/
void TcpServer::epolltimeout(EventLoop* loop)
{
	if (_timeoutcb)
	{
		_timeoutcb(loop);
	}
}

/*	删除conns_中的Connection对象，在EventLoop::handletimer()中将回调此函数	*/
void TcpServer::removeconn(int fd)
{
	{
		std::lock_guard<std::mutex> gd(_mutex);
		_connsmap.erase(fd);	// 从 map 删除 conn
	}
}

void TcpServer::setnewconnectioncb(std::function<void(spConnection)> fn)
{
	_newconnectioncb = fn;
}

void TcpServer::setcloseconnectioncb(std::function<void(spConnection)> fn)
{
	_closeconnectioncb = fn;
}

void TcpServer::seterrorconnectioncb(std::function<void(spConnection)> fn)
{
	_errorconnectioncb = fn;
}

void TcpServer::setonmessagecb(std::function<void(spConnection, std::string& message)> fn)
{
	_onmessagecb = fn;
}

void TcpServer::setsendcompletecb(std::function<void(spConnection)> fn)
{
	_sendcompletecb = fn;
}

void TcpServer::settimeoutcb(std::function<void(EventLoop*)> fn)
{
	_timeoutcb = fn;
}