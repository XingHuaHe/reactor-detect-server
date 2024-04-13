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
	std::vector<std::thread> _threads;				// �̳߳��е��߳�
	std::queue<std::function<void()>> _taskqueue;	// �������
	std::mutex _mutex;								// �������ͬ���Ļ�����
	std::condition_variable _condition;				// �������ͬ������������
	std::atomic_bool _stop;							// �����������У���stop_��ֵ����Ϊtrue��ȫ�����߳̽��˳�
	std::string _threadtype;						// �߳����ࣺ"IO"��"WORKS"
public:
	/*	�ڹ��캯���н�����threadnum���߳�	*/
	ThreadPool(size_t threadnum, const std::string& threadtype);
	/*	�����������н�ֹͣ�߳�				*/
	~ThreadPool();
	/*	��������ӵ�������					*/
	void addtask(std::function<void()> task);
	/*	�����̳߳ش�С						*/
	size_t size();
	/*	ֹͣ�̳߳�							*/
	void stop();
};