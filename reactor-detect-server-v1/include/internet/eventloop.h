#pragma once
#include "epoll.h"
#include <functional>
#include <memory>
#include <unistd.h>
#include <sys/syscall.h>
#include <queue>
#include <mutex>
#include <sys/eventfd.h>
#include "timestamp.h"
#include <sys/timerfd.h>
#include <map>
#include "connection.h"
#include <atomic>


class Epoll;
class Channel;
class Connection;
using spConnection = std::shared_ptr<Connection>;


/*  �¼�ѭ����   */
class EventLoop
{
public:
	/*  �ڹ��캯���д���Epoll����ep_  */
	EventLoop(bool mainloop, int timeval = 30, int timeout = 90);
	/*  ����������������_ep         */
	~EventLoop();

	/*  �����¼�ѭ��	*/
	void run();
	/*	ֹͣ�¼�ѭ��	*/
	void stop();
	/*	��channel���/���µ�������ϣ�channel����fd��Ҳ����Ҫ���ӵ��¼�	*/
	void updatechannel(Channel* channel);
	/*	�Ӻ�����ɾ��channel		*/
	void removechannel(Channel* channel);
	/*	����epoll_wait()��ʱ�Ļص�����	*/
	void setepolltimeoutcallback(std::function<void(EventLoop*)> fn);

	/*	�жϵ�ǰ�߳��Ƿ�Ϊ�¼�ѭ���߳�	*/
	bool isinloopthread();

	/*	��������ӵ�������				*/
	void queueinloop(std::function<void()> fn);
	/*	��eventfd�����¼�ѭ���߳�		*/
	void wakeup();
	/*	�¼�ѭ���̱߳�eventfd���Ѻ�ִ�еĺ���	*/
	void handlewakeup();
	/*	������ʱִ�еĺ���				*/
	void handletimer();
	/*	��Connection���󱣴���_conns��	*/
	void newconnection(spConnection conn);
	/*	��������ΪTcpServer::removeconn()	*/
	void settimercallback(std::function<void(int)> fn);

private:
	std::unique_ptr<Epoll> _ep;
	std::function<void(EventLoop*)> _epolltimeoutcallback;	// epoll_wait()��ʱ�Ļص�����
	pid_t _threadid;	// �¼�ѭ�������߳�ID
	std::queue<std::function<void()>> _taskqueue;	//�¼�ѭ���̱߳�eventfd���Ѻ�ִ�е��������
	std::mutex _mutex;	// �������ͬ���Ļ�����
	int  _timetvl;		// ����ʱ��������λ���롣��
	int  _timeout;		// Connection����ʱ��ʱ�䣬��λ����
	int _wakeupfd;		// ���ڻ����¼�ѭ���̵߳�eventfd
	std::unique_ptr<Channel> _wakechannel;		// eventfd��Channel
	int _timerfd;		// ��ʱ�� fd
	std::unique_ptr<Channel> _timerchannel;		// ʱ����Channel
	bool _mainloop;		// true - �����¼�ѭ����false - �Ǵ��¼�ѭ��
	/*
	1�����¼�ѭ��������map<int, spConnect> conns_��������������ڸ��¼�ѭ����ȫ����Connection����
	2���������ʱ�䵽�ˣ�����conns_���ж�ÿ��Connection�����Ƿ�ʱ��
	3�������ʱ�ˣ���conns_��ɾ��Connection����
	4������Ҫ��TcpServer.conns_��ɾ��Connection����
	5��TcpServer��EventLoop��map������Ҫ������
	6������ʱ�����ͳ�ʱʱ�������
	*/
	std::map<int, spConnection> _conns;			// ��������ڸ��¼�ѭ����ȫ����Connection����
	std::function<void(int)> _timercallback;	// ɾ��TcpServer�г�ʱ��Connection���󣬽�������ΪTcpServer::removeconn()
	std::mutex _mmutex;							// ����_conns �Ļ�����
	std::atomic_bool _stop;						// �¼�ѭ����־
};

