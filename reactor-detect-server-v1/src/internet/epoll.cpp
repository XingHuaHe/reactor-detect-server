#include "epoll.h"


Epoll::Epoll()
{
	// ����epoll������������
	if ((_epollfd = epoll_create(1)) == -1)
	{
		printf("epoll_create() failed(%d).\n", errno);
		exit(-1);
	}
}

Epoll::~Epoll()
{
	close(_epollfd); // �����������йر� _epollfd
}

/*	��channel���/���µ�������ϣ�channel����fd��Ҳ����Ҫ���ӵ��¼�	*/
void Epoll::updatechannel(Channel* channel)
{
	struct epoll_event ev;
	ev.events = channel->getevents();
	ev.data.ptr = channel;

	if (channel->getinpoll()) // ���channel������
	{
		if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, channel->getfd(), &ev) == -1)
		{
			printf("epoll_ctl( EPOLL_CTL_MOD ) failed(%d).\n", errno);
			exit(-1);
		}
	}
	else // ���channel��������
	{
		if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, channel->getfd(), &ev) == -1)
		{
			printf("epoll_ctl( EPOLL_CTL_ADD ) failed(%d).\n", errno);
			exit(-1);
		}
		channel->setinepoll(); // ��channel��_inepoll��Ա����Ϊtrue
	}
}

/*	�Ӻ������ɾ��channel		*/
void Epoll::removechannel(Channel* channel)
{
	if (channel->getinpoll())	// ���channel�Ѿ��������ˡ�
	{
		if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, channel->getfd(), 0) == -1)
		{
			perror("epoll_ctl() failed.\n"); exit(-1);
		}
	}
}

/*	����epoll_wait()���ȴ��¼��ķ������ѷ������¼���vector��������	*/
std::vector<Channel*> Epoll::loop(int timeout)
{
	std::vector<Channel*>channels; // ���epoll_wait()���ص��¼�

	bzero(_events, sizeof(_events));
	int infds = epoll_wait(_epollfd, _events, sizeof(_events) / sizeof(struct epoll_event), timeout);

	// return error.
	if (infds < 0)
	{
		// ��Ҫ���ֵĴ����� 4 �֣�ǰ������ϵͳ����
		// EBADF ��epfd����һ����Ч��������
		// EFAULT ������eventsָ����ڴ����򲻿�д
		// EINVAL ��epfd����һ��epoll�ļ������������߲���maxeventsС�ڵ���0
		// EINTR �����������б��ź��жϣ�epoll_pwait()���Ա��⣬���ߴ������У�����error�����µ���epoll_wait()
		// ��Reactorģ���У�������ʹ���źţ���Ϊ�źŴ����������鷳��û�б�Ҫ��------ ��˶
		perror("epoll_wait() failed.");
		exit(-1);
	}

	// over times.
	if (infds == 0)
	{
		return channels;
	}

	// ��� infds>0����ʾ���¼������� fd ������
	// ���ص���ѭ����ֵ�����������⣬ϵͳ���Ż�
	for (int i = 0; i < infds; i++)
	{
		Channel* ch = (Channel*)_events[i].data.ptr;
		ch->setrevents(_events[i].events);
		channels.push_back(ch);
	}

	return channels;
}
