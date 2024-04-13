#pragma once
#include "inetaddress.h"
#include <unistd.h>
#include <netinet/tcp.h>


// 创建一个非阻塞的socket
int createnonblocking();

class Socket
{
public:
	Socket(int fd);
	~Socket();

	int fd() const;                             // 返回fd_成员
	std::string getip() const;                  // 返回ip_成员
	uint16_t getport() const;                   // 返回ip_成员
	void setipport(const std::string& ip, uint16_t port);   // 设置ip_和port_成员
	void setreuseaddr(bool on);                 // 设置SO_REUSEADDR选项(true-打开，false-关闭),服务器关闭连接后，允许其端口立即被重新使用
	void setreuseport(bool on);                 // 设置SO_REUSEPORT选项,允许多个套接字（socket）绑定到同一个IP地址和端口组合上
	void settcpnodelay(bool on);                // 设置TCP_NODELAY选项,用于控制是否启用Nagle算法,当TCP_NODELAY参数被禁用时，Nagle算法会被启用
	void setkeepalive(bool on);                 // 设置SO_KEEPALIVE选项,维护每一个TCP连接，确保对方还活着
	void bind(const InetAddress& servaddr);     // 服务端的socket将调用此函数
	void listen(int n = 128);                   // 服务端的socket将调用此函数
	int accept(InetAddress& clientaddr);        // 服务端的socket将调用此函数

private:
	const int _fd;          // Socket持有的fd，在构造函数中传进来
	std::string _ip;        // 如果是listenfd，存放服务端监听的ip，如果是客户端连接的fd，存放对端的ip
	uint16_t _port;         // 如果是listenfd，存放服务端监听的port，如果是客户端连接的fd，存放外部端口
};
