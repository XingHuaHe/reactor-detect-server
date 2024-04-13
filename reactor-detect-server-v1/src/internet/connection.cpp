#include "connection.h"


Connection::Connection(EventLoop* loop , std::unique_ptr<Socket> clientsock):
	_loop(loop), _clientsock(std::move(clientsock)), _disconnect(false), _clientchannel(new Channel(_loop, _clientsock->fd()))
{
	// 为新客户端连接准备读事件，并添加到epoll中
	//_clientchannel = new Channel(_loop, _clientsock->fd());
	_clientchannel->setreadcallback(std::bind(&Connection::onmessage, this));
	_clientchannel->setclosecallback(std::bind(&Connection::closecallback, this));
	_clientchannel->seterrorcallback(std::bind(&Connection::errorcallback, this));
	_clientchannel->setwritecallback(std::bind(&Connection::writecallback, this));
	_clientchannel->enablereading(); // 让epoll_wait()监视clientchannel的读事件
	_clientchannel->setet(); // 客户端连上来的fd采用边缘触发
}

Connection::~Connection()
{
	//delete _clientsock;
	//delete _clientchannel; 
	//printf("~Connection (fd=%d) ", fd());
}

// 返回fd_成员
int Connection::fd() const
{
	return _clientsock->fd();
}

// 返回ip_成员
std::string Connection::getip() const
{
	return _clientsock->getip();
}

// 返回ip_成员
uint16_t Connection::getport() const
{
	return _clientsock->getport();
}

/*	处理对端发送过来的消息，已连接的客户端有事件	*/
void Connection::onmessage()
{
	char buffer[1024];
	while (true)
	{
		bzero(&buffer, sizeof(buffer));
		ssize_t nread = read(fd(), buffer, sizeof(buffer));
		if (nread > 0)
		{
			_inputbuffer.append(buffer, nread);
		}
		else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取
		{
			continue;
		}
		else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕
		{
			std::string message;
			while (true)
			{	
				if (_inputbuffer.pickmessage(message) == false)
				{
					break;
				}
				//printf("recv(eventfd=%d):%s\n", fd(), message.c_str());

				_lastatime = Timestamp::now();	// 修改时间戳

				_onmessagecallback(shared_from_this(), message);	// 回调TcpServer::onmessage()
			}
			break;
		}
		else if (nread == 0) // 客户端连接已断开
		{
			closecallback();
			break;
		}
	}
}

/*	TCP连接关闭（断开）的回调函数，供Channel回调	*/
void Connection::closecallback()
{
	_disconnect = true;
	_clientchannel->remove();	// 从事件循环中删除Channel
	_closecallback(shared_from_this());
}

/*	TCP连接错误的回调函数，供Channel回调			*/
void Connection::errorcallback()
{
	_disconnect = true;
	_clientchannel->remove();	// 从事件循环中删除Channel
	_errorcallback(shared_from_this());
}

/*	处理写事件的回调函数，供Channel回调				*/
void Connection::writecallback()
{
	int writen = ::send(fd(), _outputbuffer.data(), _outputbuffer.size(), 0);	// 尝试把outputbuffer_中的数据全部发送出去
	if (writen > 0) _outputbuffer.erase(0, writen);		// 从outputbuffer_中删除已成功发送的字节数

	// 如果发送缓冲区中没有数据了，表示数据已发送完成，不再关注写事件
	if (_outputbuffer.size() == 0)
	{
		_clientchannel->disablewriting();
		// 发送完成
		_sendcompletecallback(shared_from_this());
	}
}

/*	设置关闭fd_的回调函数			*/
void Connection::setclosecallback(std::function<void(spConnection)> fn)
{
	_closecallback = fn;
}

/*	设置fd_发生了错误的回调函数		*/
void Connection::seterrorcallback(std::function<void(spConnection)> fn)
{
	_errorcallback = fn;
}

/*	设置处理报文的回调函数			*/
void Connection::setonmessagecallback(std::function<void(spConnection, std::string&)> fn)
{
	_onmessagecallback = fn;
}

/*	设置发送数据完成后的回调函数	*/
void Connection::setsendcompletecallback(std::function<void(spConnection)> fn)
{
	_sendcompletecallback = fn;
}

/*	发送数据，不管是 IO 还是 WORKS 线程，都要调用 send	*/
void Connection::send(const char* data, size_t size)
{
	// 判断是否已经断开
	if (_disconnect == true)
	{
		printf("client disconnect，send() return.\n");
		return;
	}

	// 判断当前线程是否为事件循环线程（IO线程）
	if (_loop->isinloopthread()) // 如果当前线程是IO线程，直接调用sendinloop()发送数据
	{
		sendinloop(data, size);
	}
	else // 如果当前线程不是IO线程，调用EventLoop::queueinloop()，把sendinloop()交给事件循环线程去执行
	{
		_loop->queueinloop(std::bind(&Connection::sendinloop, this, data, size));
		//_loop->queueinloop(std::bind(&Connection::sendinloop, shared_from_this(), data, size));
	}
}

/*	发送数据，如果当前线程是 IO 线程，直接调用该函数，如果是 WORKS 线程，需要将该函数传给 IO 线程	*/
void Connection::sendinloop(const char* data, size_t size)
{
	_outputbuffer.appendwithseq(data, size);	// 把需要发送的数据保存到 Connection 的发送缓冲区中
	_clientchannel->enablewriting();			// 注册写事件
}

/*	判断TCP连接是否超时（空闲太久）	*/
bool Connection::timeout(time_t now, int val)
{
	return (now - _lastatime.toint()) > val;
}