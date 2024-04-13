#pragma once
#include "socket.h"
#include <functional>
#include "eventloop.h"
#include "connection.h"



class EventLoop;


class Channel
{
public:
	/*	Epoll* ep:		epoll.
		int fd:			socket fd.
		bool islisten:	server listen channel is true, client channel is false.	*/
	Channel(EventLoop* loop, int fd);
	~Channel();

	/*	����_fd��Ա		*/
	int getfd();
	/*	���ñ�Ե����	*/
	void setet();
	/*	�� epoll_wait() ���� _fd �Ķ��¼���ע����¼���	*/
	void enablereading();
	/*	ȡ�����¼�	*/
	void disablereading();
	/*	ע��д�¼�	*/
	void enablewriting();
	/*	ȡ��д�¼�	*/
	void disablewriting();
	/*	ȡ��ȫ���¼�	*/
	void disableall();
	/*	���¼�ѭ����ɾ��Channel	*/
	void remove();
	/*	���� _inepoll ��Ա��ֵ����Ϊ true	*/
	void setinepoll();
	/*	����_inepoll��Ա	*/
	bool getinpoll();
	/*	����_revents��Ա��ֵ����Ϊev	*/
	void setrevents(uint32_t ev);
	/*	����_revents��Ա	*/
	uint32_t getrevents();
	/*	����_events��Ա		*/
	uint32_t getevents();

	/*	�¼���������epoll_wait()���ص�ʱ��ִ����	*/
	void handleevent();
	/*	����_fd���¼��Ļص�����	*/
	void setreadcallback(std::function<void()> fn);
	/*	���ùر�fd_�Ļص�����	*/
	void setclosecallback(std::function<void()> fn);
	/*	����fd_�����˴���Ļص�����	*/
	void seterrorcallback(std::function<void()> fn);
	/*	����д�¼��Ļص�����	*/
	void setwritecallback(std::function<void()> fn);

private:
	int _fd = -1;               // Channelӵ�е�fd��Channel��fd��һ��һ�Ĺ�ϵ
	EventLoop* _loop;// = nullptr;
	//const std::unique_ptr<EventLoop>& _loop = nullptr;	// Channel��Ӧ�ĺ������Channel��EventLoop�Ƕ��һ�Ĺ�ϵ��һ��Channelֻ��Ӧһ��EventLoop
	bool _inepoll = false;      // Channel�Ƿ�����ӵ�epoll���ϣ����δ��ӣ�����epoll_ctl()��ʱ����EPOLL_CTL_ADD��������EPOLL_CTL_MOD
	uint32_t _events = 0;       // _fd��Ҫ���ӵ��¼���listenfd��clientfd��Ҫ����EPOLLIN��clientfd��������Ҫ����EPOLLOUT
	uint32_t _revents = 0;      // _fd�ѷ������¼�
	std::function<void()> _readcallback;		// _fd ���¼��Ļص�����
	std::function<void()> _closecallback;		// �ر�fd_�Ļص����������ص�Connection::closecallback()
	std::function<void()> _errorcallback;		// fd_�����˴���Ļص����������ص�Connection::errorcallback()
	std::function<void()> _writecallback;		// fd_д�¼��Ļص����������ص�Connection::writecallback()
};
