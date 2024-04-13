#include "eventloop.h"

int createtimerfd(int sec = 30)
{
	int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);	// ����timerfd
	struct itimerspec timeout;						// ��ʱʱ������ݽṹ
	memset(&timeout, 0, sizeof(struct itimerspec));
	timeout.it_value.tv_sec = sec;					// ��ʱʱ�䣬�̶�Ϊ5���������
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
	_wakechannel->enablereading(); // ע����¼�

	_timerchannel->setreadcallback(std::bind(&EventLoop::handletimer, this));
	_timerchannel->enablereading();
}

EventLoop::~EventLoop()
{
	//delete _ep;
}

/*  �����¼�ѭ��	*/
void EventLoop::run()
{
	//printf("EventLoop::run() thread is %ld.\n", syscall(SYS_gettid));
	_threadid = syscall(SYS_gettid);

	// event circle.
	while (!_stop)
	{
		std::vector<Channel*> channels = _ep->loop(10 * 100000);	// �ȴ����ӵ� fd ���¼�����

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

/*	ֹͣ�¼�ѭ��	*/
void EventLoop::stop()
{
	_stop = true;
	// epoll ѭ��������ñ�־��������ֻ�е���ʱ�ȲŻᴥ��������Ҫ��������
	wakeup();
}

/*	��channel���/���µ�������ϣ�channel����fd��Ҳ����Ҫ���ӵ��¼�	*/
void EventLoop::updatechannel(Channel* channel)
{
	_ep->updatechannel(channel);
}

/*	�Ӻ�����ɾ��channel		*/
void EventLoop::removechannel(Channel* channel)
{
	_ep->removechannel(channel);
}


/*	����epoll_wait()��ʱ�Ļص�����	*/
void EventLoop::setepolltimeoutcallback(std::function<void(EventLoop*)> fn)
{
	_epolltimeoutcallback = fn;
}

/*	�жϵ�ǰ�߳��Ƿ�Ϊ�¼�ѭ���߳�	*/
bool EventLoop::isinloopthread()
{
	//printf("_threadid=%d,  SYS_gettid=%ld\n", _threadid, syscall(SYS_gettid));
	return _threadid == syscall(SYS_gettid);
}

/*	��������ӵ�������				*/
void EventLoop::queueinloop(std::function<void()> fn)
{
	{
		std::lock_guard<std::mutex> gd(_mutex);	// ��������м���
		_taskqueue.push(fn);
		//_taskqueue.emplace(fn);
	}
	wakeup();	// �����¼�ѭ��
}

/*	��eventfd�����¼�ѭ���߳�		*/
void EventLoop::wakeup()
{
	uint64_t val = 1;
	write(_wakeupfd, &val, sizeof(val)); // ����
}

/*	�¼�ѭ���̱߳�eventfd���Ѻ�ִ�еĺ���	*/
void EventLoop::handlewakeup()
{
	//printf("handlewakeup() thread id is %ld.\n", syscall(SYS_gettid));
	uint64_t val;

	read(_wakeupfd, &val, sizeof(val));	// ��eventfd�ж�ȡ�����ݣ��������ȡ��eventfd�Ķ��¼���һֱ����

	std::function<void()> fn;

	std::lock_guard<std::mutex> gd(_mutex);	// ������м���

	// ִ�ж�����ȫ���ķ�������
	while (_taskqueue.size() > 0)
	{
		fn = std::move(_taskqueue.front());	// ����һ��Ԫ��
		//fn = _taskqueue.front();
		_taskqueue.pop();
		fn();	// ִ������
	}
}

/*	������ʱִ�еĺ���				*/
void EventLoop::handletimer()
{
	// ���¼�ʱ��
	struct itimerspec timeout;						// ��ʱʱ������ݽṹ��
	memset(&timeout, 0, sizeof(struct itimerspec));
	timeout.it_value.tv_sec = _timeout;				// ��ʱʱ�䣬�̶�Ϊ5��������ԡ�
	timeout.it_value.tv_nsec = 0;
	timerfd_settime(_timerfd, 0, &timeout, 0);

	if (_mainloop)
	{
		//printf("mainloop timer alarm (pid=%ld).\n", syscall(SYS_gettid));
		// ���¼�ѭ��û�� Connection ����
	}
	else
	{
		//printf("subloop timer alarm (pid=%ld).", syscall(SYS_gettid));
		time_t now = time(0);	// ��ȡ��ǰʱ��
		/*
		for (auto aa:conns_)
        {
            printf(" %d",aa.first);
            if (aa.second->timeout(now,timeout_)) 
            {
                printf("EventLoop::handletimer()1  thread is %d.\n",syscall(SYS_gettid)); 
                {
                    std::lock_guard<std::mutex> gd(mmutex_);
                    conns_.erase(aa.first);               // ��EventLoop��map��ɾ����ʱ��conn��
                }
                timercallback_(aa.first);             // ��TcpServer��map��ɾ����ʱ��conn��
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
			_conns.erase(tmp[i]);	// ��EventLoop��map��ɾ����ʱ��conn
			_timercallback(tmp[i]);	// ��TcpServer��map��ɾ����ʱ��conn
		}
		//printf("\n");
	}
}

/*	��Connection���󱣴���_conns��	*/
void EventLoop::newconnection(spConnection conn)
{
	{
		std::lock_guard<std::mutex> gd(_mmutex);
		_conns[conn->fd()] = conn;
	}
}

/*	��������ΪTcpServer::removeconn()	*/
void EventLoop::settimercallback(std::function<void(int)> fn)
{
	_timercallback = fn;
}
