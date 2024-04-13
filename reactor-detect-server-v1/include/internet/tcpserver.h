#pragma once
#include "eventloop.h"
#include "acceptor.h"
#include "connection.h"
#include "socket.h"
#include <map>
#include "threadpool.h"
#include <mutex>


class TcpServer
{
public:
	TcpServer(const uint16_t port, int threadnum = 5);
	~TcpServer();

	/*	运行事件循环	*/
	void start();
	/*	停止时间循环	*/
	void stop();
	/*	处理新客户端连接请求	*/
	void newconnection(std::unique_ptr<Socket> clientsock);
	/*	关闭客户端的连接，在Connection类中回调此函数	*/
	void closeconnection(spConnection conn);
	/*	客户端的连接错误，在Connection类中回调此函数	*/
	void errorconnection(spConnection conn);
	/*	处理客户端的请求报文，在Connection类中回调此函数	*/
	void onmessage(spConnection conn, std::string& message);
	/*	数据发送完成后，在Connection类中回调此函数			*/
	void sendcomplete(spConnection conn);
	/*	epoll_wait()超时，在EventLoop类中回调此函数			*/
	void epolltimeout(EventLoop* loop);
	/*	删除conns_中的Connection对象，在EventLoop::handletimer()中将回调此函数	*/
	void removeconn(int fd);

private:
	std::unique_ptr<EventLoop> _mainloop;	// 一个TcpServer可以有多个事件循环，现在是单线程，暂时只用一个事件循环
	std::vector<std::unique_ptr<EventLoop>> _subloops;		// 存放从事件循环的容器
	Acceptor _acceptor;						// 一个TcpServer只有一个Acceptor对象（可以采用栈对象）
	int _threadnum;							// 线程池的大小，即从事件循环的个数
	ThreadPool _threadpool;					// 线程池（因为只有一个线程池，可以不用指针）
	std::map<int, spConnection>	_connsmap;	// 一个TcpServer有多个Connection对象，存放在map容器中
	std::mutex _mutex;						// 保护_connsmap的互斥锁

public:
	std::function<void(spConnection)> _newconnectioncb;			// 回调EchoServer::HandleNewConnection()
	std::function<void(spConnection)> _closeconnectioncb;		// 回调EchoServer::HandleClose()
	std::function<void(spConnection)> _errorconnectioncb;		// 回调EchoServer::HandleError()
	std::function<void(spConnection, std::string& message)> _onmessagecb;	// 回调EchoServer::HandleMessage()
	std::function<void(spConnection)> _sendcompletecb;			// 回调EchoServer::HandleSendComplete()
	std::function<void(EventLoop*)>  _timeoutcb;				// 回调EchoServer::HandleTimeOut()
	void setnewconnectioncb(std::function<void(spConnection)> fn);
	void setcloseconnectioncb(std::function<void(spConnection)> fn);
	void seterrorconnectioncb(std::function<void(spConnection)> fn);
	void setonmessagecb(std::function<void(spConnection, std::string& message)> fn);
	void setsendcompletecb(std::function<void(spConnection)> fn);
	void settimeoutcb(std::function<void(EventLoop*)> fn);
};

