#pragma once
#include "tcpserver.h"
#include "threadpool.h"


class EchoServer
{
public:
	EchoServer(const uint16_t port, int subthreadnum = 5, int workthreadnum = 5);
	~EchoServer();

	/*	启动服务	*/
	void Start();
	/*	停止服务	*/
	void Stop();

	/*	处理新客户端连接请求，在TcpServer类中回调此函数		*/
	void HandleNewConnection(spConnection conn);
	/*	关闭客户端的连接，在TcpServer类中回调此函数			*/
	void HandleClose(spConnection conn);
	/*	客户端的连接错误，在TcpServer类中回调此函数			*/
	void HandleError(spConnection conn);
	/*	处理客户端的请求报文，在TcpServer类中回调此函数		*/
	void HandleMessage(spConnection conn, std::string& message);
	/*	数据发送完成后，在TcpServer类中回调此函数			*/
	void HandleSendComplete(spConnection conn);
	/*	epoll_wait()超时，在TcpServer类中回调此函数			*/
	void HandleTimeOut(EventLoop *loop);

	/*	处理客户端的请求报文，用于添加给线程池				*/
	void OnMessage(spConnection conn, std::string& message);

private:
	TcpServer _tcpserver;
	int _workthreadnum;			// 工作线程池的大小
	ThreadPool _threadpool;		// 工作线程池（因为 EchoServer HandleMessage函数进行计算，故此工作线程在该类中）
};

