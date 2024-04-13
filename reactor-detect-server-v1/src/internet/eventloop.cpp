#include "eventloop.h"

int createtimerfd(int sec = 30)
{
	int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);	// 创建timerfd
	struct itimerspec timeout;						// 定时时间的数据结构
	memset(&timeout, 0, sizeof(struct itimerspec));
	timeout.it_value.tv_sec = sec;					// 定时时间，固定为5，方便测试
	timeout.it_value.tv_nsec = 0;
	timerfd_settime(tfd, 0, &timeout, 0);
	return tfd;
}


EventLoop::EventLoop(bool mainloop, int timeval, int timeout):
	_ep(new Epoll()),
	_timetvl(timeval),
	_timeout(timeout),
	_wakeupfd(eventfd(0, EFD_NONBLOCK)),
	_wakechannel(new Channel(this, _wakeupfd)),
	_timerfd(createtimerfd(_timeout)),
	_timerchannel(new Channel(this, _timerfd)),
	_mainloop(mainloop),
	_stop(false)
{
	_wakechannel->setreadcallback(std::bind(&EventLoop::handlewakeup, this));
	_wakechannel->enablereading(); // 注册读事件

	_timerchannel->setreadcallback(std::bind(&EventLoop::handletimer, this));
	_timerchannel->enablereading();
}

EventLoop::~EventLoop()
{
	//delete _ep;
}

/*  运行事件循环	*/
void EventLoop::run()
{
	//printf("EventLoop::run() thread is %ld.\n", syscall(SYS_gettid));
	_threadid = syscall(SYS_gettid);

	// event circle.
	while (!_stop)
	{
		std::vector<Channel*> channels = _ep->loop(10 * 100000);	// 等待监视的 fd 有事件发生

		if (channels.size() == 0)
		{
			_epolltimeoutcallback(this);
		}

		for (auto& ch : channels)
		{
			ch->handleevent();
		}
	}
}

/*	停止事件循环	*/
void EventLoop::stop()
{
	_stop = true;
	// epoll 循环不会因该标志而触发，只有当超时等才会触发，所以要主动唤醒
	wakeup();
}

/*	把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件	*/
void EventLoop::updatechannel(Channel* channel)
{
	_ep->updatechannel(channel);
}

/*	从黑树上删除channel		*/
void EventLoop::removechannel(Channel* channel)
{
	_ep->removechannel(channel);
}


/*	设置epoll_wait()超时的回调函数	*/
void EventLoop::setepolltimeoutcallback(std::function<void(EventLoop*)> fn)
{
	_epolltimeoutcallback = fn;
}

/*	判断当前线程是否为事件循环线程	*/
bool EventLoop::isinloopthread()
{
	//printf("_threadid=%d,  SYS_gettid=%ld\n", _threadid, syscall(SYS_gettid));
	return _threadid == syscall(SYS_gettid);
}

/*	把任务添加到队列中				*/
void EventLoop::queueinloop(std::function<void()> fn)
{
	{
		std::lock_guard<std::mutex> gd(_mutex);	// 给任务队列加锁
		_taskqueue.push(fn);
		//_taskqueue.emplace(fn);
	}
	wakeup();	// 唤醒事件循环
}

/*	用eventfd唤醒事件循环线程		*/
void EventLoop::wakeup()
{
	uint64_t val = 1;
	write(_wakeupfd, &val, sizeof(val)); // 唤醒
}

/*	事件循环线程被eventfd唤醒后执行的函数	*/
void EventLoop::handlewakeup()
{
	//printf("handlewakeup() thread id is %ld.\n", syscall(SYS_gettid));
	uint64_t val;

	read(_wakeupfd, &val, sizeof(val));	// 从eventfd中读取出数据，如果不读取，eventfd的读事件会一直触发

	std::function<void()> fn;

	std::lock_guard<std::mutex> gd(_mutex);	// 任务队列加锁

	// 执行队列中全部的发送任务
	while (_taskqueue.size() > 0)
	{
		fn = std::move(_taskqueue.front());	// 出队一个元素
		//fn = _taskqueue.front();
		_taskqueue.pop();
		fn();	// 执行任务
	}
}

/*	闹钟响时执行的函数				*/
void EventLoop::handletimer()
{
	// 重新计时。
	struct itimerspec timeout;						// 定时时间的数据结构。
	memset(&timeout, 0, sizeof(struct itimerspec));
	timeout.it_value.tv_sec = _timeout;				// 定时时间，固定为5，方便测试。
	timeout.it_value.tv_nsec = 0;
	timerfd_settime(_timerfd, 0, &timeout, 0);

	if (_mainloop)
	{
		//printf("mainloop timer alarm (pid=%ld).\n", syscall(SYS_gettid));
		// 主事件循环没有 Connection 对象
	}
	else
	{
		//printf("subloop timer alarm (pid=%ld).", syscall(SYS_gettid));
		time_t now = time(0);	// 获取当前时间
		/*
		for (auto aa:conns_)
        {
            printf(" %d",aa.first);
            if (aa.second->timeout(now,timeout_)) 
            {
                printf("EventLoop::handletimer()1  thread is %d.\n",syscall(SYS_gettid)); 
                {
                    std::lock_guard<std::mutex> gd(mmutex_);
                    conns_.erase(aa.first);               // 从EventLoop的map中删除超时的conn。
                }
                timercallback_(aa.first);             // 从TcpServer的map中删除超时的conn。
            }
        }
		*/
		std::vector<int> tmp;
		//for (auto it : _conns)
		for(std::map<int, spConnection>::iterator it = _conns.begin(); it != _conns.end(); it++)
		{
			//printf(" %d", it->first);
			if (it->second->timeout(now, _timeout))
			{
				tmp.push_back(it->first);
			}
		}
		std::lock_guard<std::mutex> gd(_mmutex);
		for (int i = 0; i < tmp.size(); i++)
		{
			_conns.erase(tmp[i]);	// 从EventLoop的map中删除超时的conn
			_timercallback(tmp[i]);	// 从TcpServer的map中删除超时的conn
		}
		//printf("\n");
	}
}

/*	把Connection对象保存在_conns中	*/
void EventLoop::newconnection(spConnection conn)
{
	{
		std::lock_guard<std::mutex> gd(_mmutex);
		_conns[conn->fd()] = conn;
	}
}

/*	将被设置为TcpServer::removeconn()	*/
void EventLoop::settimercallback(std::function<void(int)> fn)
{
	_timercallback = fn;
}
