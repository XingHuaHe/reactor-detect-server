#include "inetaddress.h"


InetAddress::InetAddress()
{

}

// 如果是监听的fd，采用这个构造函数
InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
	memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	_addr.sin_port = htons(port);
	
}

// 如果是监听的fd，采用这个构造函数
InetAddress::InetAddress(uint16_t port)
{
	memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_addr.sin_port = htons(port);
}

// 如果是客户端连上来的fd，用这个构造函数
InetAddress::InetAddress(const sockaddr_in addr)
{
	_addr = addr;
}

InetAddress::~InetAddress()
{
}

// 返回字符串表示的地址，例如：192.168.150.128
const char* InetAddress::ip() const
{
	return inet_ntoa(_addr.sin_addr);
}

// 返回整数表示的端口，例如：80
uint16_t InetAddress::port() const
{
	return ntohs(_addr.sin_port);
}

// 返回 _addr 成员的地址，转换成了sockaddr
const sockaddr* InetAddress::addr() const
{
	return (sockaddr*)&_addr;
}

void InetAddress::setaddr(sockaddr_in addr)
{
	_addr = addr;
}