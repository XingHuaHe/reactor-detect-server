#include "socket.h"


int createnonblocking()
{
	int servsock = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (servsock < 0)
	{
		printf("%s:%s:%d listen socket create error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
		exit(-1);
	}

	return servsock;
}

Socket::Socket(int fd): _fd(fd)
{
}

Socket::~Socket()
{
	::close(_fd);
}

// 返回fd_成员
int Socket::fd() const
{
	return _fd;
}

/*	返回ip_成员	*/
std::string Socket::getip() const
{
	return _ip;
}

/*	返回ip_成员	*/
uint16_t Socket::getport() const
{
	return _port;
}

/*	设置ip_和port_成员	*/
void Socket::setipport(const std::string& ip, uint16_t port)
{
	_ip = ip;
	_port = port;
}

// 设置SO_REUSEADDR选项，true-打开，false-关闭
void Socket::setreuseaddr(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

// 设置SO_REUSEPORT选项
void Socket::setreuseport(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

// 设置TCP_NODELAY选项
void Socket::settcpnodelay(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
}

// 设置SO_KEEPALIVE选项
void Socket::setkeepalive(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}


// 服务端的socket将调用此函数
void Socket::bind(const InetAddress& servaddr)
{
	if (::bind(_fd, servaddr.addr(), sizeof(sockaddr)) < 0)
	{
		perror("bind() failed");
		close(_fd);
		exit(-1);
	}
	setipport(servaddr.ip(), servaddr.port());
}

// 服务端的socket将调用此函数
void Socket::listen(int n)
{
	if (::listen(_fd, n) != 0)
	{
		perror("listen() failed");
		close(_fd);
		exit(-1);
	}
}

// 服务端的socket将调用此函数
int Socket::accept(InetAddress& clientaddr)
{
	struct sockaddr_in peeraddr;
	socklen_t len = sizeof(peeraddr);
	int clntfd = accept4(_fd, (struct sockaddr*)&peeraddr, &len, SOCK_NONBLOCK); // 替代 setnonblocking

	clientaddr.setaddr(peeraddr);

	return clntfd;
}