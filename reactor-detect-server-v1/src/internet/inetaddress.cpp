#include "inetaddress.h"


InetAddress::InetAddress()
{

}

// ����Ǽ�����fd������������캯��
InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
	memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	_addr.sin_port = htons(port);
	
}

// ����Ǽ�����fd������������캯��
InetAddress::InetAddress(uint16_t port)
{
	memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_addr.sin_port = htons(port);
}

// ����ǿͻ�����������fd����������캯��
InetAddress::InetAddress(const sockaddr_in addr)
{
	_addr = addr;
}

InetAddress::~InetAddress()
{
}

// �����ַ�����ʾ�ĵ�ַ�����磺192.168.150.128
const char* InetAddress::ip() const
{
	return inet_ntoa(_addr.sin_addr);
}

// ����������ʾ�Ķ˿ڣ����磺80
uint16_t InetAddress::port() const
{
	return ntohs(_addr.sin_port);
}

// ���� _addr ��Ա�ĵ�ַ��ת������sockaddr
const sockaddr* InetAddress::addr() const
{
	return (sockaddr*)&_addr;
}

void InetAddress::setaddr(sockaddr_in addr)
{
	_addr = addr;
}