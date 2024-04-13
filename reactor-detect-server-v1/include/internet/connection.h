#pragma once
#include <functional>
#include "socket.h"
#include "inetaddress.h"
#include "channel.h"
#include "eventloop.h"
#include "buffer.h"
#include <memory>
#include <atomic>
#include "timestamp.h"


class Connection;
using spConnection = std::shared_ptr<Connection>;

class EventLoop;
class Socket;
class Channel;
/*
this ָ��������ָͨ�룬������������������ָ����this���´��󣬿���ͨ���̳� enable_shared_from_this ģ����
ͨ�� 
*/
class Connection: public std::enable_shared_from_this<Connection>
{
public:
	Connection(EventLoop* loop, std::unique_ptr<Socket> clientsock);
	~Connection();

	/*	����fd_��Ա	*/
	int fd() const;
	/*	����ip_��Ա	*/
	std::string getip() const;
	/*	����ip_��Ա	*/
	uint16_t getport() const;

	/*	����Զ˷��͹�������Ϣ	*/
	void onmessage();

	/*	TCP���ӹرգ��Ͽ����Ļص���������Channel�ص�	*/
	void closecallback();
	/*	TCP���Ӵ���Ļص���������Channel�ص�			*/
	void errorcallback();
	/*	����д�¼��Ļص���������Channel�ص�				*/
	void writecallback();

	/*	���ùر�fd_�Ļص�����			*/
	void setclosecallback(std::function<void(spConnection)> fn);
	/*	����fd_�����˴���Ļص�����		*/
	void seterrorcallback(std::function<void(spConnection)> fn);
	/*	���ô����ĵĻص�����			*/
	void setonmessagecallback(std::function<void(spConnection, std::string&)> fn);
	/*	���÷���������ɺ�Ļص�����	*/
	void setsendcompletecallback(std::function<void(spConnection)> fn);

	/*	�������ݣ������� IO ���� WORKS �̣߳���Ҫ���� send	*/
	void send(const char* data, size_t size);
	/*	�������ݣ������ǰ�߳��� IO �̣߳�ֱ�ӵ��øú���������� WORKS �̣߳���Ҫ���ú������� IO �߳�	*/
	void sendinloop(const char* data, size_t size);
	/*	�ж�TCP�����Ƿ�ʱ������̫�ã�	*/
	bool timeout(time_t now, int val);

private:
	EventLoop* _loop;
	std::unique_ptr<Socket> _clientsock;
	std::unique_ptr<Channel> _clientchannel;	// ʹ�ö��ڴ棬��Ϊһ���������������ϰ���� clientchannel
	Buffer _inputbuffer;		// ���ջ�����
	Buffer _outputbuffer;		// ���ͻ�����
	std::atomic_bool _disconnect;	// �ͻ��������Ƿ��ѶϿ�������ѶϿ���������Ϊtrue (IO�̻߳�ı��ֵ��WORKS �̻߳��жϸ�ֵ��

	std::function<void(spConnection)> _closecallback;	// �ر�fd_�Ļص����������ص�TcpServer::closeconnection()
	std::function<void(spConnection)> _errorcallback;	// fd_�����˴���Ļص����������ص�TcpServer::errorconnection()
	std::function<void(spConnection, std::string&)> _onmessagecallback;	// �����ĵĻص����������ص�TcpServer::onmessage()
	std::function<void(spConnection)> _sendcompletecallback;			// ����������ɺ�Ļص����������ص�TcpServer::sendcomplete()

	Timestamp _lastatime;		// ʱ���������Connection����ʱΪ��ǰʱ�䣬ÿ���յ�һ�����ģ���ʱ�������Ϊ��ǰʱ��
};
