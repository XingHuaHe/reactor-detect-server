#pragma once
#include <arpa/inet.h>
#include <string.h>
#include <string>


class InetAddress
{
public:
	InetAddress();
	InetAddress(const std::string& ip, uint16_t port);	// 如果是监听的fd，采用这个构造函数
	InetAddress(uint16_t port);							// 如果是监听的fd，采用这个构造函数
	InetAddress(const sockaddr_in addr);				// 如果是客户端连上来的fd，用这个构造函数
	~InetAddress();

	const char* ip() const;			// 返回字符串表示的地址，例如：192.168.150.128
	uint16_t port() const;			// 返回整数表示的端口，例如：80
	const sockaddr* addr() const;	// 返回 _addr 成员的地址，转换成了sockaddr
	void setaddr(sockaddr_in addr);

private:
	sockaddr_in _addr;			// 表示地址协议的结构体
};
