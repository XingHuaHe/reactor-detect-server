#pragma once
#include "eventloop.h"
#include "acceptor.h"
#include "connection.h"
#include "socket.h"
#include <map>
#include "threadpool.h"
#include <mutex>


class TcpServer
{
public:
	TcpServer(const uint16_t port, int threadnum = 5);
	~TcpServer();

	/*	�����¼�ѭ��	*/
	void start();
	/*	ֹͣʱ��ѭ��	*/
	void stop();
	/*	�����¿ͻ�����������	*/
	void newconnection(std::unique_ptr<Socket> clientsock);
	/*	�رտͻ��˵����ӣ���Connection���лص��˺���	*/
	void closeconnection(spConnection conn);
	/*	�ͻ��˵����Ӵ�����Connection���лص��˺���	*/
	void errorconnection(spConnection conn);
	/*	����ͻ��˵������ģ���Connection���лص��˺���	*/
	void onmessage(spConnection conn, std::string& message);
	/*	���ݷ�����ɺ���Connection���лص��˺���			*/
	void sendcomplete(spConnection conn);
	/*	epoll_wait()��ʱ����EventLoop���лص��˺���			*/
	void epolltimeout(EventLoop* loop);
	/*	ɾ��conns_�е�Connection������EventLoop::handletimer()�н��ص��˺���	*/
	void removeconn(int fd);

private:
	std::unique_ptr<EventLoop> _mainloop;	// һ��TcpServer�����ж���¼�ѭ���������ǵ��̣߳���ʱֻ��һ���¼�ѭ��
	std::vector<std::unique_ptr<EventLoop>> _subloops;		// ��Ŵ��¼�ѭ��������
	Acceptor _acceptor;						// һ��TcpServerֻ��һ��Acceptor���󣨿��Բ���ջ����
	int _threadnum;							// �̳߳صĴ�С�������¼�ѭ���ĸ���
	ThreadPool _threadpool;					// �̳߳أ���Ϊֻ��һ���̳߳أ����Բ���ָ�룩
	std::map<int, spConnection>	_connsmap;	// һ��TcpServer�ж��Connection���󣬴����map������
	std::mutex _mutex;						// ����_connsmap�Ļ�����

public:
	std::function<void(spConnection)> _newconnectioncb;			// �ص�EchoServer::HandleNewConnection()
	std::function<void(spConnection)> _closeconnectioncb;		// �ص�EchoServer::HandleClose()
	std::function<void(spConnection)> _errorconnectioncb;		// �ص�EchoServer::HandleError()
	std::function<void(spConnection, std::string& message)> _onmessagecb;	// �ص�EchoServer::HandleMessage()
	std::function<void(spConnection)> _sendcompletecb;			// �ص�EchoServer::HandleSendComplete()
	std::function<void(EventLoop*)>  _timeoutcb;				// �ص�EchoServer::HandleTimeOut()
	void setnewconnectioncb(std::function<void(spConnection)> fn);
	void setcloseconnectioncb(std::function<void(spConnection)> fn);
	void seterrorconnectioncb(std::function<void(spConnection)> fn);
	void setonmessagecb(std::function<void(spConnection, std::string& message)> fn);
	void setsendcompletecb(std::function<void(spConnection)> fn);
	void settimeoutcb(std::function<void(EventLoop*)> fn);
};

