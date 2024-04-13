#include "threadpool.h"
// ps -T -p id

/*	�ڹ��캯���н�����threadnum���߳�	*/
ThreadPool::ThreadPool(size_t threadnum, const std::string& threadtype): _stop(false), _threadtype(threadtype)
{
	// ����threadnum���̣߳�ÿ���߳̽�����������������
	for (size_t i = 0; i < threadnum; i++)
	{
		// ��lambda�������߳�
		_threads.emplace_back([this]
			{
				//printf("create %s thread(%ld).\n", _threadtype.c_str(), syscall(SYS_gettid));	// ��ʾ�߳�ID
				// std::cout << "���̣߳�" << std::this_thread::get_id() << std::endl; // C++ 11 ���׼��������

				while (_stop == false)
				{
					std::function<void()> task;	// ���ڴ�ų��ӵ�Ԫ��

					{	// ��������Ŀ�ʼ ///////////////////////////////////
						std::unique_lock<std::mutex> lock(this->_mutex);

						// �ȴ������ߵ���������
						this->_condition.wait(lock, [this]
							{
								return ((this->_stop == true) || (this->_taskqueue.empty() == false));
							});

						// ���̳߳�ֹ֮ͣǰ����������л�������ִ�������˳�
						if ((this->_stop == true) && (this->_taskqueue.empty() == true)) return;

						// ����һ������
						task = std::move(this->_taskqueue.front());
						this->_taskqueue.pop();
					}   // ��������Ľ��� ///////////////////////////////////

					//printf("%s (%ld) execute task.\n", _threadtype.c_str(), syscall(SYS_gettid));
					task();	// ִ������
				}
			});
	}
}

/*	�����������н�ֹͣ�߳�				*/
ThreadPool::~ThreadPool()
{
	stop();
}

/*	��������ӵ�������					*/
void ThreadPool::addtask(std::function<void()> task)
{
	{   // ��������Ŀ�ʼ ///////////////////////////////////
		std::lock_guard<std::mutex> lock(_mutex);
		_taskqueue.push(task);
	}   // ��������Ľ��� ///////////////////////////////////

	_condition.notify_one();	// ����һ���߳�
}

/*	�����̳߳ش�С						*/
size_t ThreadPool::size()
{
	return _threads.size();
}

/*	ֹͣ�̳߳�							*/
void ThreadPool::stop()
{
	if (_stop)
	{
		return;
	}
	_stop = true;

	_condition.notify_all();	// ����ȫ�����߳�

	// �ȴ�ȫ���߳�ִ����������˳�
	for (std::thread& th : _threads)
	{
		th.join();
	}
}