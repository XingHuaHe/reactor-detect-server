#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <sys/syscall.h>
#include <mutex>
#include <unistd.h>
#include <thread>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>

class ThreadPool
{
private:
	std::vector<std::thread> _threads;				// 线程池中的线程
	std::queue<std::function<void()>> _taskqueue;	// 任务队列
	std::mutex _mutex;								// 任务队列同步的互斥锁
	std::condition_variable _condition;				// 任务队列同步的条件变量
	std::atomic_bool _stop;							// 在析构函数中，把stop_的值设置为true，全部的线程将退出
	std::string _threadtype;						// 线程种类："IO"、"WORKS"
public:
	/*	在构造函数中将启动threadnum个线程	*/
	ThreadPool(size_t threadnum, const std::string& threadtype);
	/*	在析构函数中将停止线程				*/
	~ThreadPool();
	/*	把任务添加到队列中					*/
	void addtask(std::function<void()> task);
	/*	返回线程池大小						*/
	size_t size();
	/*	停止线程池							*/
	void stop();
};