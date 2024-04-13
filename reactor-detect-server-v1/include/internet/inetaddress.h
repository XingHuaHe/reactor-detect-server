#pragma once
#include <arpa/inet.h>
#include <string.h>
#include <string>


class InetAddress
{
public:
	InetAddress();
	InetAddress(const std::string& ip, uint16_t port);	// ����Ǽ�����fd������������캯��
	InetAddress(uint16_t port);							// ����Ǽ�����fd������������캯��
	InetAddress(const sockaddr_in addr);				// ����ǿͻ�����������fd����������캯��
	~InetAddress();

	const char* ip() const;			// �����ַ�����ʾ�ĵ�ַ�����磺192.168.150.128
	uint16_t port() const;			// ����������ʾ�Ķ˿ڣ����磺80
	const sockaddr* addr() const;	// ���� _addr ��Ա�ĵ�ַ��ת������sockaddr
	void setaddr(sockaddr_in addr);

private:
	sockaddr_in _addr;			// ��ʾ��ַЭ��Ľṹ��
};
