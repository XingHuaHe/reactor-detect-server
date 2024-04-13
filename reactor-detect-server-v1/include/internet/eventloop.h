#pragma once
#include "epoll.h"
#include <functional>
#include <memory>
#include <unistd.h>
#include <sys/syscall.h>
#include <queue>
#include <mutex>
#include <sys/eventfd.h>
#include "timestamp.h"
#include <sys/timerfd.h>
#include <map>
#include "connection.h"
#include <atomic>


class Epoll;
class Channel;
class Connection;
using spConnection = std::shared_ptr<Connection>;


/*  事件循环类   */
class EventLoop
{
public:
	/*  在构造函数中创建Epoll对象ep_  */
	EventLoop(bool mainloop, int timeval = 30, int timeout = 90);
	/*  在析构函数中销毁_ep         */
	~EventLoop();

	/*  运行事件循环	*/
	void run();
	/*	停止事件循环	*/
	void stop();
	/*	把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件	*/
	void updatechannel(Channel* channel);
	/*	从黑树上删除channel		*/
	void removechannel(Channel* channel);
	/*	设置epoll_wait()超时的回调函数	*/
	void setepolltimeoutcallback(std::function<void(EventLoop*)> fn);

	/*	判断当前线程是否为事件循环线程	*/
	bool isinloopthread();

	/*	把任务添加到队列中				*/
	void queueinloop(std::function<void()> fn);
	/*	用eventfd唤醒事件循环线程		*/
	void wakeup();
	/*	事件循环线程被eventfd唤醒后执行的函数	*/
	void handlewakeup();
	/*	闹钟响时执行的函数				*/
	void handletimer();
	/*	把Connection对象保存在_conns中	*/
	void newconnection(spConnection conn);
	/*	将被设置为TcpServer::removeconn()	*/
	void settimercallback(std::function<void(int)> fn);

private:
	std::unique_ptr<Epoll> _ep;
	std::function<void(EventLoop*)> _epolltimeoutcallback;	// epoll_wait()超时的回调函数
	pid_t _threadid;	// 事件循环所在线程ID
	std::queue<std::function<void()>> _taskqueue;	//事件循环线程被eventfd唤醒后执行的任务队列
	std::mutex _mutex;	// 任务队列同步的互斥锁
	int  _timetvl;		// 闹钟时间间隔，单位：秒。。
	int  _timeout;		// Connection对象超时的时间，单位：秒
	int _wakeupfd;		// 用于唤醒事件循环线程的eventfd
	std::unique_ptr<Channel> _wakechannel;		// eventfd的Channel
	int _timerfd;		// 定时器 fd
	std::unique_ptr<Channel> _timerchannel;		// 时器的Channel
	bool _mainloop;		// true - 是主事件循环，false - 是从事件循环
	/*
	1、在事件循环中增加map<int, spConnect> conns_容器，存放运行在该事件循环上全部的Connection对象。
	2、如果闹钟时间到了，遍历conns_，判断每个Connection对象是否超时。
	3、如果超时了，从conns_中删除Connection对象；
	4、还需要从TcpServer.conns_中删除Connection对象。
	5、TcpServer和EventLoop的map容器需要加锁。
	6、闹钟时间间隔和超时时间参数化
	*/
	std::map<int, spConnection> _conns;			// 存放运行在该事件循环上全部的Connection对象
	std::function<void(int)> _timercallback;	// 删除TcpServer中超时的Connection对象，将被设置为TcpServer::removeconn()
	std::mutex _mmutex;							// 保护_conns 的互斥锁
	std::atomic_bool _stop;						// 事件循环标志
};

