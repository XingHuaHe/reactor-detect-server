#pragma once
#include "eventloop.h"
#include "socket.h"
#include "channel.h"
#include <functional>


class Acceptor
{
public:
	Acceptor(EventLoop* loop, uint16_t port);
	~Acceptor();

	/*	处理新客户端连接请求	*/
	void newconnection();
	/*	设置处理新客户端连接请求的回调函数，将在创建Acceptor对象的时候（TcpServer类的构造函数中）设置	*/
	void setnewconnectioncb(std::function<void(std::unique_ptr<Socket>)> fn);

private:
	EventLoop* _loop;	// Acceptor对应的事件循环，在构造函数中传入（因为没有管理权，故此不能使用 std::move）
	Socket _servsock;			// 服务端用于监听的socket，在构造函数中创建
	Channel _acceptchannel;		// Acceptor对应的channel，在构造函数中创建（使用栈内存，因为一个服务程序只有一个 acceptchannel）
	std::function<void(std::unique_ptr<Socket>)> _newconnectioncb;	// 处理新客户端连接请求的回调函数，将指向TcpServer::newconnection()

};

