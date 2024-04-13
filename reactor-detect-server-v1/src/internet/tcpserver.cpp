#include "tcpserver.h"



TcpServer::TcpServer(const uint16_t port, int threadnum):
	_threadnum(threadnum), _mainloop(new EventLoop(true, 30, 90)), _acceptor(_mainloop.get(), port), _threadpool(_threadnum, "IO")
{
	//_mainloop = new EventLoop();
	_mainloop->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));

	//_acceptor = new Acceptor(_mainloop, port);
	_acceptor.setnewconnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1));

	// �����̳߳�
	//_threadpool = new ThreadPool(_threadnum, "IO");

	// �������¼�"ѭ��"
	for (int i = 0; i < _threadnum; i++)
	{
		_subloops.emplace_back(new EventLoop(false, 30, 90));
		//_subloops.push_back(new EventLoop());
		_subloops[i]->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1)); // ���ó�ʱ
		_subloops[i]->settimercallback(std::bind(&TcpServer::removeconn, this, std::placeholders::_1));	// �����������TCP���ӵĻص�����
		// ������д��¼����� run ������Ϊ�����Ӹ��̳߳�
		_threadpool.addtask(std::bind(&EventLoop::run, _subloops[i].get()));	// ���̳߳������д��¼�ѭ��
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

	//for (auto& it : _subloops)	// �ͷŴ��¼�
	//{
	//	delete it;
	//}

	//delete _threadpool;	//�ͷ��̳߳�
}

/*	�����¼�ѭ��	*/
void TcpServer::start()
{
	_mainloop->run();
}

/*	ֹͣʱ��ѭ��	*/
void TcpServer::stop()
{
	// ֹͣ���¼�ѭ��
	_mainloop->stop();
	printf("mainloop stopped!\n");
	// ֹͣ���¼�ѭ��
	for (int i = 0; i < _threadnum; i++)
	{
		_subloops[i]->stop();
	}
	printf("subloops stopped!\n");
	// ֹͣ IO �߳�
	_threadpool.stop();
	printf("threadpool stopped!\n");
}

/*	�����¿ͻ�����������	*/
void TcpServer::newconnection(std::unique_ptr<Socket> clientsock)
{
	/*	Connection ���������ڣ��ͻ������ӣ��������󣻿ͻ��˶Ͽ������ٶ���
		TcpServer �ܹ�֪��ʲôʱ��ͻ������ӣ�ʲôʱ��Ͽ����ʴˣ�
		Connection Ӧ������ TcpServer	*/
	spConnection conn(new Connection(_subloops[clientsock->fd() % _threadnum].get(), std::move(clientsock)));
	//Connection* conn = new Connection(_subloops[clientsock->fd()%_threadnum], clientsock);	// ����ȡ�����ѡ����¼�ѭ��
	//Connection* conn = new Connection(_mainloop, clientsock);	// ָ������mainloop�¼�ѭ���ִ���
	conn->setclosecallback(std::bind(&TcpServer::closeconnection, this, std::placeholders::_1));
	conn->seterrorcallback(std::bind(&TcpServer::errorconnection, this, std::placeholders::_1));
	conn->setonmessagecallback(std::bind(&TcpServer::onmessage, this, std::placeholders::_1, std::placeholders::_2));
	conn->setsendcompletecallback(std::bind(&TcpServer::sendcomplete, this, std::placeholders::_1));

	{
		std::lock_guard<std::mutex> gd(_mutex);
		_connsmap[conn->fd()] = conn;	// �� conn ����� TcpServer �� map������
	}
	_subloops[conn->fd() % _threadnum]->newconnection(conn);	// ��conn��ŵ�EventLoop��map������

	if (_newconnectioncb)
	{
		_newconnectioncb(conn);
	}
}

/*	�رտͻ��˵����ӣ���Connection���лص��˺���	*/
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
	��������̻߳�δ�����꣬�ͻ��˹رյ��� TcpServer::closeconnection �����ã�
	��ô����� conn Ұָ����֣����³������
	����취�� Connection ��������ָ��
	*/
	//delete conn;
}

/*	�ͻ��˵����Ӵ�����Connection���лص��˺���	*/
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

/*	����ͻ��˵������ģ���Connection���лص��˺���	*/
void TcpServer::onmessage(spConnection conn, std::string& message)
{
	if (_onmessagecb)
	{
		_onmessagecb(conn, message);
	}
}

/*	���ݷ�����ɺ���Connection���лص��˺���			*/
void TcpServer::sendcomplete(spConnection conn)
{
	if (_sendcompletecb)
	{
		_sendcompletecb(conn);
	}
}

/*	epoll_wait()��ʱ����EventLoop���лص��˺���			*/
void TcpServer::epolltimeout(EventLoop* loop)
{
	if (_timeoutcb)
	{
		_timeoutcb(loop);
	}
}

/*	ɾ��conns_�е�Connection������EventLoop::handletimer()�н��ص��˺���	*/
void TcpServer::removeconn(int fd)
{
	{
		std::lock_guard<std::mutex> gd(_mutex);
		_connsmap.erase(fd);	// �� map ɾ�� conn
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