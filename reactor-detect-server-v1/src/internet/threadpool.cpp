#include "threadpool.h"
// ps -T -p id

/*	在构造函数中将启动threadnum个线程	*/
ThreadPool::ThreadPool(size_t threadnum, const std::string& threadtype): _stop(false), _threadtype(threadtype)
{
	// 启动threadnum个线程，每个线程将阻塞在条件变量上
	for (size_t i = 0; i < threadnum; i++)
	{
		// 用lambda函创建线程
		_threads.emplace_back([this]
			{
				//printf("create %s thread(%ld).\n", _threadtype.c_str(), syscall(SYS_gettid));	// 显示线程ID
				// std::cout << "子线程：" << std::this_thread::get_id() << std::endl; // C++ 11 后标准，不建议

				while (_stop == false)
				{
					std::function<void()> task;	// 用于存放出队的元素

					{	// 锁作用域的开始 ///////////////////////////////////
						std::unique_lock<std::mutex> lock(this->_mutex);

						// 等待生产者的条件变量
						this->_condition.wait(lock, [this]
							{
								return ((this->_stop == true) || (this->_taskqueue.empty() == false));
							});

						// 在线程池停止之前，如果队列中还有任务，执行完再退出
						if ((this->_stop == true) && (this->_taskqueue.empty() == true)) return;

						// 出队一个任务
						task = std::move(this->_taskqueue.front());
						this->_taskqueue.pop();
					}   // 锁作用域的结束 ///////////////////////////////////

					//printf("%s (%ld) execute task.\n", _threadtype.c_str(), syscall(SYS_gettid));
					task();	// 执行任务
				}
			});
	}
}

/*	在析构函数中将停止线程				*/
ThreadPool::~ThreadPool()
{
	stop();
}

/*	把任务添加到队列中					*/
void ThreadPool::addtask(std::function<void()> task)
{
	{   // 锁作用域的开始 ///////////////////////////////////
		std::lock_guard<std::mutex> lock(_mutex);
		_taskqueue.push(task);
	}   // 锁作用域的结束 ///////////////////////////////////

	_condition.notify_one();	// 唤醒一个线程
}

/*	返回线程池大小						*/
size_t ThreadPool::size()
{
	return _threads.size();
}

/*	停止线程池							*/
void ThreadPool::stop()
{
	if (_stop)
	{
		return;
	}
	_stop = true;

	_condition.notify_all();	// 唤醒全部的线程

	// 等待全部线程执行完任务后退出
	for (std::thread& th : _threads)
	{
		th.join();
	}
}