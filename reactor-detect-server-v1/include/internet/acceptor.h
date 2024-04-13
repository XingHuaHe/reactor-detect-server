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

	/*	�����¿ͻ�����������	*/
	void newconnection();
	/*	���ô����¿ͻ�����������Ļص����������ڴ���Acceptor�����ʱ��TcpServer��Ĺ��캯���У�����	*/
	void setnewconnectioncb(std::function<void(std::unique_ptr<Socket>)> fn);

private:
	EventLoop* _loop;	// Acceptor��Ӧ���¼�ѭ�����ڹ��캯���д��루��Ϊû�й���Ȩ���ʴ˲���ʹ�� std::move��
	Socket _servsock;			// ��������ڼ�����socket���ڹ��캯���д���
	Channel _acceptchannel;		// Acceptor��Ӧ��channel���ڹ��캯���д�����ʹ��ջ�ڴ棬��Ϊһ���������ֻ��һ�� acceptchannel��
	std::function<void(std::unique_ptr<Socket>)> _newconnectioncb;	// �����¿ͻ�����������Ļص���������ָ��TcpServer::newconnection()

};

