#pragma once
#include <functional>
#include "socket.h"
#include "inetaddress.h"
#include "channel.h"
#include "eventloop.h"
#include "buffer.h"
#include <memory>
#include <atomic>
#include "timestamp.h"


class Connection;
using spConnection = std::shared_ptr<Connection>;

class EventLoop;
class Socket;
class Channel;
/*
this 指针属于普通指针，如果在类里面调用智能指针因this导致错误，可以通过继承 enable_shared_from_this 模板类
通过 
*/
class Connection: public std::enable_shared_from_this<Connection>
{
public:
	Connection(EventLoop* loop, std::unique_ptr<Socket> clientsock);
	~Connection();

	/*	返回fd_成员	*/
	int fd() const;
	/*	返回ip_成员	*/
	std::string getip() const;
	/*	返回ip_成员	*/
	uint16_t getport() const;

	/*	处理对端发送过来的消息	*/
	void onmessage();

	/*	TCP连接关闭（断开）的回调函数，供Channel回调	*/
	void closecallback();
	/*	TCP连接错误的回调函数，供Channel回调			*/
	void errorcallback();
	/*	处理写事件的回调函数，供Channel回调				*/
	void writecallback();

	/*	设置关闭fd_的回调函数			*/
	void setclosecallback(std::function<void(spConnection)> fn);
	/*	设置fd_发生了错误的回调函数		*/
	void seterrorcallback(std::function<void(spConnection)> fn);
	/*	设置处理报文的回调函数			*/
	void setonmessagecallback(std::function<void(spConnection, std::string&)> fn);
	/*	设置发送数据完成后的回调函数	*/
	void setsendcompletecallback(std::function<void(spConnection)> fn);

	/*	发送数据，不管是 IO 还是 WORKS 线程，都要调用 send	*/
	void send(const char* data, size_t size);
	/*	发送数据，如果当前线程是 IO 线程，直接调用该函数，如果是 WORKS 线程，需要将该函数传给 IO 线程	*/
	void sendinloop(const char* data, size_t size);
	/*	判断TCP连接是否超时（空闲太久）	*/
	bool timeout(time_t now, int val);

private:
	EventLoop* _loop;
	std::unique_ptr<Socket> _clientsock;
	std::unique_ptr<Channel> _clientchannel;	// 使用堆内存，因为一个服务器程序有上百万个 clientchannel
	Buffer _inputbuffer;		// 接收缓冲区
	Buffer _outputbuffer;		// 发送缓冲区
	std::atomic_bool _disconnect;	// 客户端连接是否已断开，如果已断开，则设置为true (IO线程会改变该值，WORKS 线程会判断该值）

	std::function<void(spConnection)> _closecallback;	// 关闭fd_的回调函数，将回调TcpServer::closeconnection()
	std::function<void(spConnection)> _errorcallback;	// fd_发生了错误的回调函数，将回调TcpServer::errorconnection()
	std::function<void(spConnection, std::string&)> _onmessagecallback;	// 处理报文的回调函数，将回调TcpServer::onmessage()
	std::function<void(spConnection)> _sendcompletecallback;			// 发送数据完成后的回调函数，将回调TcpServer::sendcomplete()

	Timestamp _lastatime;		// 时间戳，创建Connection对象时为当前时间，每接收到一个报文，把时间戳更新为当前时间
};
