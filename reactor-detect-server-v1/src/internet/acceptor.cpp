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
	//_acceptchannel = new Channel(_loop, _servsock.fd()); // 这里 new 出来的对象没有释放，这个问题以后再解决
	/*	std::bind() 绑定类成员函数时，
		第一个参数为对象成员函数指针（编译器不会将对象成员函数隐式转化为函数指针，需要手动 & 转换），
		第二个参数为对象的地址（因为对象成员函数需要 this 指针
		第三个参数为成员函数参数	*/
	_acceptchannel.setreadcallback(std::bind(&Acceptor::newconnection, this));
	_acceptchannel.enablereading(); // 让 epoll_wait() 监视 _acceptchannel 的读事件
}

Acceptor::~Acceptor()
{
	//delete _servsock;
	//delete _acceptchannel;
}

/*	处理新客户端连接请求，servsock 有事件，表示有新的客户端连上来	*/
void Acceptor::newconnection()
{
	InetAddress clientaddr;
	// 注意，clientsock只能new出来，不能在栈上，否则析构函数会关闭fd。
	// 还有，这里new出来的对象没有释放，这个问题以后再解决。
	std::unique_ptr<Socket> clientsock(new Socket(_servsock.accept(clientaddr)));
	clientsock->setipport(clientaddr.ip(), clientaddr.port());

	_newconnectioncb(std::move(clientsock));	// 回调 TcpServer newconnection
}

/*	设置处理新客户端连接请求的回调函数，将在创建Acceptor对象的时候（TcpServer类的构造函数中）设置	*/
void Acceptor::setnewconnectioncb(std::function<void(std::unique_ptr<Socket>)> fn)
{
	_newconnectioncb = fn;
}