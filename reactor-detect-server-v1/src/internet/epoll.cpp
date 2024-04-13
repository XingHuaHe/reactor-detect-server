#include "epoll.h"


Epoll::Epoll()
{
	// 创建epoll句柄（红黑树）
	if ((_epollfd = epoll_create(1)) == -1)
	{
		printf("epoll_create() failed(%d).\n", errno);
		exit(-1);
	}
}

Epoll::~Epoll()
{
	close(_epollfd); // 在析构函数中关闭 _epollfd
}

/*	把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件	*/
void Epoll::updatechannel(Channel* channel)
{
	struct epoll_event ev;
	ev.events = channel->getevents();
	ev.data.ptr = channel;

	if (channel->getinpoll()) // 如果channel在树上
	{
		if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, channel->getfd(), &ev) == -1)
		{
			printf("epoll_ctl( EPOLL_CTL_MOD ) failed(%d).\n", errno);
			exit(-1);
		}
	}
	else // 如果channel不在树上
	{
		if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, channel->getfd(), &ev) == -1)
		{
			printf("epoll_ctl( EPOLL_CTL_ADD ) failed(%d).\n", errno);
			exit(-1);
		}
		channel->setinepoll(); // 把channel的_inepoll成员设置为true
	}
}

/*	从红黑树上删除channel		*/
void Epoll::removechannel(Channel* channel)
{
	if (channel->getinpoll())	// 如果channel已经在树上了。
	{
		if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, channel->getfd(), 0) == -1)
		{
			perror("epoll_ctl() failed.\n"); exit(-1);
		}
	}
}

/*	运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回	*/
std::vector<Channel*> Epoll::loop(int timeout)
{
	std::vector<Channel*>channels; // 存放epoll_wait()返回的事件

	bzero(_events, sizeof(_events));
	int infds = epoll_wait(_epollfd, _events, sizeof(_events) / sizeof(struct epoll_event), timeout);

	// return error.
	if (infds < 0)
	{
		// 主要出现的错误有 4 种：前三个是系统错误
		// EBADF ：epfd不是一个有效的描述符
		// EFAULT ：参数events指向的内存区域不可写
		// EINVAL ：epfd不是一个epoll文件描述符，或者参数maxevents小于等于0
		// EINTR ：阻塞过程中被信号中断，epoll_pwait()可以避免，或者错误处理中，解析error后重新调用epoll_wait()
		// 在Reactor模型中，不建议使用信号，因为信号处理起来很麻烦，没有必要。------ 陈硕
		perror("epoll_wait() failed.");
		exit(-1);
	}

	// over times.
	if (infds == 0)
	{
		return channels;
	}

	// 如果 infds>0，表示有事件发生的 fd 的数量
	// 不必担心循环赋值导致性能问题，系统会优化
	for (int i = 0; i < infds; i++)
	{
		Channel* ch = (Channel*)_events[i].data.ptr;
		ch->setrevents(_events[i].events);
		channels.push_back(ch);
	}

	return channels;
}
