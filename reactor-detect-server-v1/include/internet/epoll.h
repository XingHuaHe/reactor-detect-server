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

	/*	把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件	*/
	void updatechannel(Channel* channel);
	/*	从红黑树上删除channel		*/
	void removechannel(Channel* channel);
	/*	运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回	*/
	std::vector<Channel*> loop(int timeout = -1);

private:
	static const int EventsNums = 128;		// epoll_wait()返回事件数组的大小
	int _epollfd = -1;						// epoll句柄，在构造函数中创建
	epoll_event _events[EventsNums];		// 存放poll_wait()返回事件的数组，在构造函数中分配内存
};
