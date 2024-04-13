#include "echoserver.h"


EchoServer::EchoServer(const uint16_t port, int subthreadnum, int workthreadnum)
	:_tcpserver(port, subthreadnum), _workthreadnum(workthreadnum), _threadpool(workthreadnum, "WORKS")
{
	// 以下代码不是必须的，业务关心什么事件，就指定相应的回调函数。
	_tcpserver.setnewconnectioncb(std::bind(&EchoServer::HandleNewConnection, this, std::placeholders::_1));
	_tcpserver.setcloseconnectioncb(std::bind(&EchoServer::HandleClose, this, std::placeholders::_1));
	_tcpserver.seterrorconnectioncb(std::bind(&EchoServer::HandleError, this, std::placeholders::_1));
	_tcpserver.setonmessagecb(std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
	_tcpserver.setsendcompletecb(std::bind(&EchoServer::HandleSendComplete, this, std::placeholders::_1));
	_tcpserver.settimeoutcb(std::bind(&EchoServer::HandleTimeOut, this, std::placeholders::_1));
}


EchoServer::~EchoServer()
{
}

/*	启动服务	*/
void EchoServer::Start()
{
	_tcpserver.start();
}

/*	停止服务	*/
void EchoServer::Stop()
{
	// 停止工作线程
	_threadpool.stop();
	printf("stop WORKS threads.\n");

	// 停止 IO 线程
	_tcpserver.stop();
}

/*	处理新客户端连接请求，在TcpServer类中回调此函数		*/
void EchoServer::HandleNewConnection(spConnection conn)
{
	printf("%s new client connection (fd=%d,ip=%s, port=%d) ok.\n",Timestamp::now().tostring().c_str(), conn->fd(), conn->getip().c_str(), conn->getport());

	// 根据业务的需求，在这里可以增加其它的代码
}
/*	关闭客户端的连接，在TcpServer类中回调此函数			*/
void EchoServer::HandleClose(spConnection conn)
{
	printf("%s client(eventfd=%d) disconnected.\n", Timestamp::now().tostring().c_str(), conn->fd());

	// 根据业务的需求，在这里可以增加其它的代码
}

/*	客户端的连接错误，在TcpServer类中回调此函数			*/
void EchoServer::HandleError(spConnection conn)
{
	printf("client(eventfd=%d) error.\n", conn->fd());

	// 根据业务的需求，在这里可以增加其它的代码
}

/*	处理客户端的请求报文，在TcpServer类中回调此函数		*/
void EchoServer::HandleMessage(spConnection conn, std::string& message)
{
	if (_threadpool.size() == 0) // 没有启动线程池
	{
		printf("No using WORKS threads.\n");
		OnMessage(conn, message);
	}
	else // 启动工作线程
	{
		// 将计算业务放入工作线程池处理
		printf("Using WORKS threads.\n");
		_threadpool.addtask(std::bind(&EchoServer::OnMessage, this, conn, message));
	}
}

/*	处理客户端的请求报文，用于添加给线程池				*/
void EchoServer::OnMessage(spConnection conn, std::string& message)
{
	/*	在这里，将经过若干步骤的运算	*/
	message = "reply:" + message;
	conn->send(message.data(), message.size());
}

/*	数据发送完成后，在TcpServer类中回调此函数			*/
void EchoServer::HandleSendComplete(spConnection conn)
{
	//printf("message send complete.\n");

	// 根据业务的需求，在这里可以增加其它的代码
}

/*	epoll_wait()超时，在TcpServer类中回调此函数			*/
void EchoServer::HandleTimeOut(EventLoop* loop)
{
	printf("epoll_wait() timeout.\n");

	// 根据业务的需求，在这里可以增加其它的代码
}
