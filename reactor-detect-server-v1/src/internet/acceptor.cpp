#include "acceptor.h"


Acceptor::Acceptor(EventLoop* loop, uint16_t port):
	_loop(loop), _servsock(createnonblocking()), _acceptchannel(_loop, _servsock.fd())
{
	// server listen address.
	InetAddress servaddr(port);

	// socket.
	_servsock.setreuseaddr(true);
	_servsock.settcpnodelay(true);
	_servsock.setreuseport(true);
	_servsock.setkeepalive(true);
	// bind.
	_servsock.bind(servaddr);
	// listen.
	_servsock.listen(128);

	// epoll.
	//EventLoop loop;
	//_acceptchannel = new Channel(_loop, _servsock.fd()); // ���� new �����Ķ���û���ͷţ���������Ժ��ٽ��
	/*	std::bind() �����Ա����ʱ��
		��һ������Ϊ�����Ա����ָ�루���������Ὣ�����Ա������ʽת��Ϊ����ָ�룬��Ҫ�ֶ� & ת������
		�ڶ�������Ϊ����ĵ�ַ����Ϊ�����Ա������Ҫ this ָ��
		����������Ϊ��Ա��������	*/
	_acceptchannel.setreadcallback(std::bind(&Acceptor::newconnection, this));
	_acceptchannel.enablereading(); // �� epoll_wait() ���� _acceptchannel �Ķ��¼�
}

Acceptor::~Acceptor()
{
	//delete _servsock;
	//delete _acceptchannel;
}

/*	�����¿ͻ�����������servsock ���¼�����ʾ���µĿͻ���������	*/
void Acceptor::newconnection()
{
	InetAddress clientaddr;
	// ע�⣬clientsockֻ��new������������ջ�ϣ���������������ر�fd��
	// ���У�����new�����Ķ���û���ͷţ���������Ժ��ٽ����
	std::unique_ptr<Socket> clientsock(new Socket(_servsock.accept(clientaddr)));
	clientsock->setipport(clientaddr.ip(), clientaddr.port());

	_newconnectioncb(std::move(clientsock));	// �ص� TcpServer newconnection
}

/*	���ô����¿ͻ�����������Ļص����������ڴ���Acceptor�����ʱ��TcpServer��Ĺ��캯���У�����	*/
void Acceptor::setnewconnectioncb(std::function<void(std::unique_ptr<Socket>)> fn)
{
	_newconnectioncb = fn;
}