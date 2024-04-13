#pragma once
#include <sys/epoll.h>
#include <vector>
#include <stdio.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "channel.h"


class Channel;

/*
epoll
*/
class Epoll
{
public:
	Epoll();
	~Epoll();

	/*	��channel���/���µ�������ϣ�channel����fd��Ҳ����Ҫ���ӵ��¼�	*/
	void updatechannel(Channel* channel);
	/*	�Ӻ������ɾ��channel		*/
	void removechannel(Channel* channel);
	/*	����epoll_wait()���ȴ��¼��ķ������ѷ������¼���vector��������	*/
	std::vector<Channel*> loop(int timeout = -1);

private:
	static const int EventsNums = 128;		// epoll_wait()�����¼�����Ĵ�С
	int _epollfd = -1;						// epoll������ڹ��캯���д���
	epoll_event _events[EventsNums];		// ���poll_wait()�����¼������飬�ڹ��캯���з����ڴ�
};
