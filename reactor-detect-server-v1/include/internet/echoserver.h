#pragma once
#include "tcpserver.h"
#include "threadpool.h"


class EchoServer
{
public:
	EchoServer(const uint16_t port, int subthreadnum = 5, int workthreadnum = 5);
	~EchoServer();

	/*	��������	*/
	void Start();
	/*	ֹͣ����	*/
	void Stop();

	/*	�����¿ͻ�������������TcpServer���лص��˺���		*/
	void HandleNewConnection(spConnection conn);
	/*	�رտͻ��˵����ӣ���TcpServer���лص��˺���			*/
	void HandleClose(spConnection conn);
	/*	�ͻ��˵����Ӵ�����TcpServer���лص��˺���			*/
	void HandleError(spConnection conn);
	/*	����ͻ��˵������ģ���TcpServer���лص��˺���		*/
	void HandleMessage(spConnection conn, std::string& message);
	/*	���ݷ�����ɺ���TcpServer���лص��˺���			*/
	void HandleSendComplete(spConnection conn);
	/*	epoll_wait()��ʱ����TcpServer���лص��˺���			*/
	void HandleTimeOut(EventLoop *loop);

	/*	����ͻ��˵������ģ�������Ӹ��̳߳�				*/
	void OnMessage(spConnection conn, std::string& message);

private:
	TcpServer _tcpserver;
	int _workthreadnum;			// �����̳߳صĴ�С
	ThreadPool _threadpool;		// �����̳߳أ���Ϊ EchoServer HandleMessage�������м��㣬�ʴ˹����߳��ڸ����У�
};

