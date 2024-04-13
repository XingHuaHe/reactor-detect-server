#pragma once
#include "inetaddress.h"
#include <unistd.h>
#include <netinet/tcp.h>


// ����һ����������socket
int createnonblocking();

class Socket
{
public:
	Socket(int fd);
	~Socket();

	int fd() const;                             // ����fd_��Ա
	std::string getip() const;                  // ����ip_��Ա
	uint16_t getport() const;                   // ����ip_��Ա
	void setipport(const std::string& ip, uint16_t port);   // ����ip_��port_��Ա
	void setreuseaddr(bool on);                 // ����SO_REUSEADDRѡ��(true-�򿪣�false-�ر�),�������ر����Ӻ�������˿�����������ʹ��
	void setreuseport(bool on);                 // ����SO_REUSEPORTѡ��,�������׽��֣�socket���󶨵�ͬһ��IP��ַ�Ͷ˿������
	void settcpnodelay(bool on);                // ����TCP_NODELAYѡ��,���ڿ����Ƿ�����Nagle�㷨,��TCP_NODELAY����������ʱ��Nagle�㷨�ᱻ����
	void setkeepalive(bool on);                 // ����SO_KEEPALIVEѡ��,ά��ÿһ��TCP���ӣ�ȷ���Է�������
	void bind(const InetAddress& servaddr);     // ����˵�socket�����ô˺���
	void listen(int n = 128);                   // ����˵�socket�����ô˺���
	int accept(InetAddress& clientaddr);        // ����˵�socket�����ô˺���

private:
	const int _fd;          // Socket���е�fd���ڹ��캯���д�����
	std::string _ip;        // �����listenfd����ŷ���˼�����ip������ǿͻ������ӵ�fd����ŶԶ˵�ip
	uint16_t _port;         // �����listenfd����ŷ���˼�����port������ǿͻ������ӵ�fd������ⲿ�˿�
};
