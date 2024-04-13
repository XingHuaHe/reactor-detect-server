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

// ����fd_��Ա
int Socket::fd() const
{
	return _fd;
}

/*	����ip_��Ա	*/
std::string Socket::getip() const
{
	return _ip;
}

/*	����ip_��Ա	*/
uint16_t Socket::getport() const
{
	return _port;
}

/*	����ip_��port_��Ա	*/
void Socket::setipport(const std::string& ip, uint16_t port)
{
	_ip = ip;
	_port = port;
}

// ����SO_REUSEADDRѡ�true-�򿪣�false-�ر�
void Socket::setreuseaddr(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

// ����SO_REUSEPORTѡ��
void Socket::setreuseport(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

// ����TCP_NODELAYѡ��
void Socket::settcpnodelay(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)); // TCP_NODELAY����ͷ�ļ� <netinet/tcp.h>
}

// ����SO_KEEPALIVEѡ��
void Socket::setkeepalive(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}


// ����˵�socket�����ô˺���
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

// ����˵�socket�����ô˺���
void Socket::listen(int n)
{
	if (::listen(_fd, n) != 0)
	{
		perror("listen() failed");
		close(_fd);
		exit(-1);
	}
}

// ����˵�socket�����ô˺���
int Socket::accept(InetAddress& clientaddr)
{
	struct sockaddr_in peeraddr;
	socklen_t len = sizeof(peeraddr);
	int clntfd = accept4(_fd, (struct sockaddr*)&peeraddr, &len, SOCK_NONBLOCK); // ��� setnonblocking

	clientaddr.setaddr(peeraddr);

	return clntfd;
}