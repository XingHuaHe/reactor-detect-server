#include "channel.h"


/*	Epoll* ep:		epoll.
	int fd:			socket fd.
	bool islisten:	server listen channel is true, client channel is false.	*/
Channel::Channel(EventLoop* loop, int fd): _loop(loop), _fd(fd)
{
}

Channel::~Channel()
{
	// �����������У���Ҫ����_loop��Ҳ���ܹر�fd_����Ϊ����������������Channel�࣬Channel��ֻ����Ҫ���ǣ�ʹ�����Ƕ���
}

/*	����_fd��Ա	*/
int Channel::getfd()
{
	return _fd;
}

/*	���ñ�Ե����	*/
void Channel::setet()
{
	_events |= EPOLLET;
}

/*	��epoll_wait()����_fd�Ķ��¼�	*/
void Channel::enablereading()
{
	_events |= EPOLLIN;
	_loop->updatechannel(this);
}

/*	ȡ�����¼�	*/
void Channel::disablereading()
{
	_events &= ~EPOLLIN;
	_loop->updatechannel(this);
}

/*	ע��д�¼�	*/
void Channel::enablewriting()
{
	_events |= EPOLLOUT;
	_loop->updatechannel(this);
}

/*	ȡ��д�¼�	*/
void Channel::disablewriting()
{
	_events &= ~EPOLLOUT;
	_loop->updatechannel(this);
}

/*	ȡ��ȫ���¼�	*/
void Channel::disableall()
{
	_events = 0;
	_loop->updatechannel(this);
}

/*	���¼�ѭ����ɾ��Channel	*/
void Channel::remove()
{
	disableall();	// ȡ��ȫ���¼�
	_loop->removechannel(this);	// �Ӻ������ɾ�� fd
}

/*	����_inepoll��Ա��ֵ����Ϊ true	*/
void Channel::setinepoll()
{
	_inepoll = true;
}

/*	����_inepoll��Ա	*/
bool Channel::getinpoll()
{
	return _inepoll;
}

/*	����_revents��Ա��ֵ����Ϊev	*/
void Channel::setrevents(uint32_t ev)
{
	_revents = ev;
}

/*	����_revents��Ա	*/
uint32_t Channel::getrevents()
{
	return _revents;
}

/*	����_events��Ա	*/
uint32_t Channel::getevents()
{
	return _events;
}

/*	�¼���������epoll_wait()���ص�ʱ��ִ����	*/
void Channel::handleevent()
{
	if (_revents & EPOLLRDHUP) // ��1���ͻ����ѹرա���Щϵͳ��ⲻ��������ʹ�� EPOLLIN��recv()����0
	{
		_closecallback();
	}
	else if (_revents & (EPOLLIN | EPOLLPRI)) // ��2�����ջ������������ݿ��Զ�
	{
		_readcallback();
	}
	else if (_revents & EPOLLOUT) // ��3����������Ҫд
	{
		_writecallback();
	}
	else // �����¼�������Ϊ����
	{
		_errorcallback();
	}
}

/*	����_fd���¼��Ļص�����	*/
void Channel::setreadcallback(std::function<void()> fn)
{
	_readcallback = fn;
}

/*	���ùر�fd_�Ļص�����	*/
void Channel::setclosecallback(std::function<void()> fn)
{
	_closecallback = fn;
}

/*	����fd_�����˴���Ļص�����	*/
void Channel::seterrorcallback(std::function<void()> fn)
{
	_errorcallback = fn;
}

/*	����д�¼��Ļص�����	*/
void Channel::setwritecallback(std::function<void()> fn)
{
	_writecallback = fn;
}