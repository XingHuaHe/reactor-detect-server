#include "channel.h"


/*	Epoll* ep:		epoll.
	int fd:			socket fd.
	bool islisten:	server listen channel is true, client channel is false.	*/
Channel::Channel(EventLoop* loop, int fd): _loop(loop), _fd(fd)
{
}

Channel::~Channel()
{
	// 在析构函数中，不要销毁_loop，也不能关闭fd_，因为这两个东西不属于Channel类，Channel类只是需要它们，使用它们而已
}

/*	返回_fd成员	*/
int Channel::getfd()
{
	return _fd;
}

/*	设置边缘触发	*/
void Channel::setet()
{
	_events |= EPOLLET;
}

/*	让epoll_wait()监视_fd的读事件	*/
void Channel::enablereading()
{
	_events |= EPOLLIN;
	_loop->updatechannel(this);
}

/*	取消读事件	*/
void Channel::disablereading()
{
	_events &= ~EPOLLIN;
	_loop->updatechannel(this);
}

/*	注册写事件	*/
void Channel::enablewriting()
{
	_events |= EPOLLOUT;
	_loop->updatechannel(this);
}

/*	取消写事件	*/
void Channel::disablewriting()
{
	_events &= ~EPOLLOUT;
	_loop->updatechannel(this);
}

/*	取消全部事件	*/
void Channel::disableall()
{
	_events = 0;
	_loop->updatechannel(this);
}

/*	从事件循环中删除Channel	*/
void Channel::remove()
{
	disableall();	// 取消全部事件
	_loop->removechannel(this);	// 从红黑树中删除 fd
}

/*	设置_inepoll成员，值设置为 true	*/
void Channel::setinepoll()
{
	_inepoll = true;
}

/*	返回_inepoll成员	*/
bool Channel::getinpoll()
{
	return _inepoll;
}

/*	设置_revents成员，值设置为ev	*/
void Channel::setrevents(uint32_t ev)
{
	_revents = ev;
}

/*	返回_revents成员	*/
uint32_t Channel::getrevents()
{
	return _revents;
}

/*	返回_events成员	*/
uint32_t Channel::getevents()
{
	return _events;
}

/*	事件处理函数，epoll_wait()返回的时候，执行它	*/
void Channel::handleevent()
{
	if (_revents & EPOLLRDHUP) // （1）客户端已关闭。有些系统检测不到，可以使用 EPOLLIN，recv()返回0
	{
		_closecallback();
	}
	else if (_revents & (EPOLLIN | EPOLLPRI)) // （2）接收缓冲区中有数据可以读
	{
		_readcallback();
	}
	else if (_revents & EPOLLOUT) // （3）有数据需要写
	{
		_writecallback();
	}
	else // 其它事件，都视为错误
	{
		_errorcallback();
	}
}

/*	设置_fd读事件的回调函数	*/
void Channel::setreadcallback(std::function<void()> fn)
{
	_readcallback = fn;
}

/*	设置关闭fd_的回调函数	*/
void Channel::setclosecallback(std::function<void()> fn)
{
	_closecallback = fn;
}

/*	设置fd_发生了错误的回调函数	*/
void Channel::seterrorcallback(std::function<void()> fn)
{
	_errorcallback = fn;
}

/*	设置写事件的回调函数	*/
void Channel::setwritecallback(std::function<void()> fn)
{
	_writecallback = fn;
}