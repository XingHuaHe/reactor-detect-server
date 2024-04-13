#include "connection.h"


Connection::Connection(EventLoop* loop , std::unique_ptr<Socket> clientsock):
	_loop(loop), _clientsock(std::move(clientsock)), _disconnect(false), _clientchannel(new Channel(_loop, _clientsock->fd()))
{
	// Ϊ�¿ͻ�������׼�����¼�������ӵ�epoll��
	//_clientchannel = new Channel(_loop, _clientsock->fd());
	_clientchannel->setreadcallback(std::bind(&Connection::onmessage, this));
	_clientchannel->setclosecallback(std::bind(&Connection::closecallback, this));
	_clientchannel->seterrorcallback(std::bind(&Connection::errorcallback, this));
	_clientchannel->setwritecallback(std::bind(&Connection::writecallback, this));
	_clientchannel->enablereading(); // ��epoll_wait()����clientchannel�Ķ��¼�
	_clientchannel->setet(); // �ͻ�����������fd���ñ�Ե����
}

Connection::~Connection()
{
	//delete _clientsock;
	//delete _clientchannel; 
	//printf("~Connection (fd=%d) ", fd());
}

// ����fd_��Ա
int Connection::fd() const
{
	return _clientsock->fd();
}

// ����ip_��Ա
std::string Connection::getip() const
{
	return _clientsock->getip();
}

// ����ip_��Ա
uint16_t Connection::getport() const
{
	return _clientsock->getport();
}

/*	����Զ˷��͹�������Ϣ�������ӵĿͻ������¼�	*/
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
		else if (nread == -1 && errno == EINTR) // ��ȡ���ݵ�ʱ���ź��жϣ�������ȡ
		{
			continue;
		}
		else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // ȫ���������Ѷ�ȡ���
		{
			std::string message;
			while (true)
			{	
				if (_inputbuffer.pickmessage(message) == false)
				{
					break;
				}
				//printf("recv(eventfd=%d):%s\n", fd(), message.c_str());

				_lastatime = Timestamp::now();	// �޸�ʱ���

				_onmessagecallback(shared_from_this(), message);	// �ص�TcpServer::onmessage()
			}
			break;
		}
		else if (nread == 0) // �ͻ��������ѶϿ�
		{
			closecallback();
			break;
		}
	}
}

/*	TCP���ӹرգ��Ͽ����Ļص���������Channel�ص�	*/
void Connection::closecallback()
{
	_disconnect = true;
	_clientchannel->remove();	// ���¼�ѭ����ɾ��Channel
	_closecallback(shared_from_this());
}

/*	TCP���Ӵ���Ļص���������Channel�ص�			*/
void Connection::errorcallback()
{
	_disconnect = true;
	_clientchannel->remove();	// ���¼�ѭ����ɾ��Channel
	_errorcallback(shared_from_this());
}

/*	����д�¼��Ļص���������Channel�ص�				*/
void Connection::writecallback()
{
	int writen = ::send(fd(), _outputbuffer.data(), _outputbuffer.size(), 0);	// ���԰�outputbuffer_�е�����ȫ�����ͳ�ȥ
	if (writen > 0) _outputbuffer.erase(0, writen);		// ��outputbuffer_��ɾ���ѳɹ����͵��ֽ���

	// ������ͻ�������û�������ˣ���ʾ�����ѷ�����ɣ����ٹ�עд�¼�
	if (_outputbuffer.size() == 0)
	{
		_clientchannel->disablewriting();
		// �������
		_sendcompletecallback(shared_from_this());
	}
}

/*	���ùر�fd_�Ļص�����			*/
void Connection::setclosecallback(std::function<void(spConnection)> fn)
{
	_closecallback = fn;
}

/*	����fd_�����˴���Ļص�����		*/
void Connection::seterrorcallback(std::function<void(spConnection)> fn)
{
	_errorcallback = fn;
}

/*	���ô����ĵĻص�����			*/
void Connection::setonmessagecallback(std::function<void(spConnection, std::string&)> fn)
{
	_onmessagecallback = fn;
}

/*	���÷���������ɺ�Ļص�����	*/
void Connection::setsendcompletecallback(std::function<void(spConnection)> fn)
{
	_sendcompletecallback = fn;
}

/*	�������ݣ������� IO ���� WORKS �̣߳���Ҫ���� send	*/
void Connection::send(const char* data, size_t size)
{
	// �ж��Ƿ��Ѿ��Ͽ�
	if (_disconnect == true)
	{
		printf("client disconnect��send() return.\n");
		return;
	}

	// �жϵ�ǰ�߳��Ƿ�Ϊ�¼�ѭ���̣߳�IO�̣߳�
	if (_loop->isinloopthread()) // �����ǰ�߳���IO�̣߳�ֱ�ӵ���sendinloop()��������
	{
		sendinloop(data, size);
	}
	else // �����ǰ�̲߳���IO�̣߳�����EventLoop::queueinloop()����sendinloop()�����¼�ѭ���߳�ȥִ��
	{
		_loop->queueinloop(std::bind(&Connection::sendinloop, this, data, size));
		//_loop->queueinloop(std::bind(&Connection::sendinloop, shared_from_this(), data, size));
	}
}

/*	�������ݣ������ǰ�߳��� IO �̣߳�ֱ�ӵ��øú���������� WORKS �̣߳���Ҫ���ú������� IO �߳�	*/
void Connection::sendinloop(const char* data, size_t size)
{
	_outputbuffer.appendwithseq(data, size);	// ����Ҫ���͵����ݱ��浽 Connection �ķ��ͻ�������
	_clientchannel->enablewriting();			// ע��д�¼�
}

/*	�ж�TCP�����Ƿ�ʱ������̫�ã�	*/
bool Connection::timeout(time_t now, int val)
{
	return (now - _lastatime.toint()) > val;
}