#include "echoserver.h"


EchoServer::EchoServer(const uint16_t port, int subthreadnum, int workthreadnum)
	:_tcpserver(port, subthreadnum), _workthreadnum(workthreadnum), _threadpool(workthreadnum, "WORKS")
{
	// ���´��벻�Ǳ���ģ�ҵ�����ʲô�¼�����ָ����Ӧ�Ļص�������
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

/*	��������	*/
void EchoServer::Start()
{
	_tcpserver.start();
}

/*	ֹͣ����	*/
void EchoServer::Stop()
{
	// ֹͣ�����߳�
	_threadpool.stop();
	printf("stop WORKS threads.\n");

	// ֹͣ IO �߳�
	_tcpserver.stop();
}

/*	�����¿ͻ�������������TcpServer���лص��˺���		*/
void EchoServer::HandleNewConnection(spConnection conn)
{
	printf("%s new client connection (fd=%d,ip=%s, port=%d) ok.\n",Timestamp::now().tostring().c_str(), conn->fd(), conn->getip().c_str(), conn->getport());

	// ����ҵ�������������������������Ĵ���
}
/*	�رտͻ��˵����ӣ���TcpServer���лص��˺���			*/
void EchoServer::HandleClose(spConnection conn)
{
	printf("%s client(eventfd=%d) disconnected.\n", Timestamp::now().tostring().c_str(), conn->fd());

	// ����ҵ�������������������������Ĵ���
}

/*	�ͻ��˵����Ӵ�����TcpServer���лص��˺���			*/
void EchoServer::HandleError(spConnection conn)
{
	printf("client(eventfd=%d) error.\n", conn->fd());

	// ����ҵ�������������������������Ĵ���
}

/*	����ͻ��˵������ģ���TcpServer���лص��˺���		*/
void EchoServer::HandleMessage(spConnection conn, std::string& message)
{
	if (_threadpool.size() == 0) // û�������̳߳�
	{
		printf("No using WORKS threads.\n");
		OnMessage(conn, message);
	}
	else // ���������߳�
	{
		// ������ҵ����빤���̳߳ش���
		printf("Using WORKS threads.\n");
		_threadpool.addtask(std::bind(&EchoServer::OnMessage, this, conn, message));
	}
}

/*	����ͻ��˵������ģ�������Ӹ��̳߳�				*/
void EchoServer::OnMessage(spConnection conn, std::string& message)
{
	/*	��������������ɲ��������	*/
	message = "reply:" + message;
	conn->send(message.data(), message.size());
}

/*	���ݷ�����ɺ���TcpServer���лص��˺���			*/
void EchoServer::HandleSendComplete(spConnection conn)
{
	//printf("message send complete.\n");

	// ����ҵ�������������������������Ĵ���
}

/*	epoll_wait()��ʱ����TcpServer���лص��˺���			*/
void EchoServer::HandleTimeOut(EventLoop* loop)
{
	printf("epoll_wait() timeout.\n");

	// ����ҵ�������������������������Ĵ���
}
