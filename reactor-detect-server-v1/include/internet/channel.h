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

	/*	返回_fd成员		*/
	int getfd();
	/*	设置边缘触发	*/
	void setet();
	/*	让 epoll_wait() 监视 _fd 的读事件（注册读事件）	*/
	void enablereading();
	/*	取消读事件	*/
	void disablereading();
	/*	注册写事件	*/
	void enablewriting();
	/*	取消写事件	*/
	void disablewriting();
	/*	取消全部事件	*/
	void disableall();
	/*	从事件循环中删除Channel	*/
	void remove();
	/*	设置 _inepoll 成员，值设置为 true	*/
	void setinepoll();
	/*	返回_inepoll成员	*/
	bool getinpoll();
	/*	设置_revents成员，值设置为ev	*/
	void setrevents(uint32_t ev);
	/*	返回_revents成员	*/
	uint32_t getrevents();
	/*	返回_events成员		*/
	uint32_t getevents();

	/*	事件处理函数，epoll_wait()返回的时候，执行它	*/
	void handleevent();
	/*	设置_fd读事件的回调函数	*/
	void setreadcallback(std::function<void()> fn);
	/*	设置关闭fd_的回调函数	*/
	void setclosecallback(std::function<void()> fn);
	/*	设置fd_发生了错误的回调函数	*/
	void seterrorcallback(std::function<void()> fn);
	/*	设置写事件的回调函数	*/
	void setwritecallback(std::function<void()> fn);

private:
	int _fd = -1;               // Channel拥有的fd，Channel和fd是一对一的关系
	EventLoop* _loop;// = nullptr;
	//const std::unique_ptr<EventLoop>& _loop = nullptr;	// Channel对应的红黑树，Channel与EventLoop是多对一的关系，一个Channel只对应一个EventLoop
	bool _inepoll = false;      // Channel是否已添加到epoll树上，如果未添加，调用epoll_ctl()的时候用EPOLL_CTL_ADD，否则用EPOLL_CTL_MOD
	uint32_t _events = 0;       // _fd需要监视的事件。listenfd和clientfd需要监视EPOLLIN，clientfd还可能需要监视EPOLLOUT
	uint32_t _revents = 0;      // _fd已发生的事件
	std::function<void()> _readcallback;		// _fd 读事件的回调函数
	std::function<void()> _closecallback;		// 关闭fd_的回调函数，将回调Connection::closecallback()
	std::function<void()> _errorcallback;		// fd_发生了错误的回调函数，将回调Connection::errorcallback()
	std::function<void()> _writecallback;		// fd_写事件的回调函数，将回调Connection::writecallback()
};
