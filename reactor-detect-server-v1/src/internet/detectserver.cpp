#include "detectserver.h"


DetectServer::DetectServer(const uint16_t port, const std::string weight, int subthreadnum, int workthreadnum)
	:_tcpserver(port, subthreadnum),
	_detector(weight),
	_workthreadnum(workthreadnum), 
	_threadpool(workthreadnum, "WORKS")
{
	// 以下代码不是必须的，业务关心什么事件，就指定相应的回调函数。
	_tcpserver.setnewconnectioncb(std::bind(&DetectServer::HandleNewConnection, this, std::placeholders::_1));
	_tcpserver.setcloseconnectioncb(std::bind(&DetectServer::HandleClose, this, std::placeholders::_1));
	_tcpserver.seterrorconnectioncb(std::bind(&DetectServer::HandleError, this, std::placeholders::_1));
	_tcpserver.setonmessagecb(std::bind(&DetectServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
	_tcpserver.setsendcompletecb(std::bind(&DetectServer::HandleSendComplete, this, std::placeholders::_1));
	_tcpserver.settimeoutcb(std::bind(&DetectServer::HandleTimeOut, this, std::placeholders::_1));

	// object detector.
	_detector.LoadClassNames(weight);	// load class names.
}


DetectServer::~DetectServer()
{
}

/*	启动服务	*/
void DetectServer::Start()
{
	_tcpserver.start();
}

/*	停止服务	*/
void DetectServer::Stop()
{
	// 停止工作线程
	_threadpool.stop();
	//printf("stop WORKS threads.\n");

	// 停止 IO 线程
	_tcpserver.stop();
}

/*	处理新客户端连接请求，在TcpServer类中回调此函数		*/
void DetectServer::HandleNewConnection(spConnection conn)
{
	printf("%s new client connection (fd=%d,ip=%s, port=%d) ok.\n",Timestamp::now().tostring().c_str(), conn->fd(), conn->getip().c_str(), conn->getport());

	// 根据业务的需求，在这里可以增加其它的代码
}
/*	关闭客户端的连接，在TcpServer类中回调此函数			*/
void DetectServer::HandleClose(spConnection conn)
{
	printf("%s client(eventfd=%d) disconnected.\n", Timestamp::now().tostring().c_str(), conn->fd());

	// 根据业务的需求，在这里可以增加其它的代码
}

/*	客户端的连接错误，在TcpServer类中回调此函数			*/
void DetectServer::HandleError(spConnection conn)
{
	printf("client(eventfd=%d) error.\n", conn->fd());

	// 根据业务的需求，在这里可以增加其它的代码
}

/*	处理客户端的请求报文，在TcpServer类中回调此函数		*/
void DetectServer::HandleMessage(spConnection conn, std::string& message)
{
	if (_threadpool.size() == 0) // 没有启动线程池
	{
		//std::cout << "message size = " << message.size() << std::endl;

		// test - save image.
		std::vector<uchar> data(message.begin(), message.end());
		cv::Mat img_tmp = cv::imdecode(data, cv::IMREAD_COLOR);
		cv::Mat img = img_tmp + 128; // restore.
		//{
		//	std::lock_guard<std::mutex> gd(_mutex);
		//}
		std::vector<std::tuple<cv::Rect, float, int>> results = _detector.Forward(img); // Inference.
		//Draw(img, results, _detector.GetClassNames());
		//cv::imwrite("/root/xinghua.he/yolov7-deploy/result1.jpg", img);

		message.clear();
		for (const auto& item : results)
		{
			cv::Rect box;
			float score;
			int class_idx;
			std::tie(box, score, class_idx) = item;
			std::string n = _detector.GetClassNames()[class_idx];

			// x,y,w,h,s,c,n
			std::ostringstream oss;
			oss << box.x << "," << box.y << "," << box.width << "," << box.height << ",";
			oss << std::to_string(score) << "," << std::to_string(class_idx) << "," << n;

			message = message + oss.str() + "\n";
		}
		//std::cout << message << std::endl;
		OnMessage(conn, message);
	}
	else // 启动工作线程
	{
		// 将计算业务放入工作线程池处理
		_threadpool.addtask(std::bind(&DetectServer::OnMessage, this, conn, message));
	}
}

/*	处理客户端的请求报文，用于添加给线程池				*/
void DetectServer::OnMessage(spConnection conn, std::string& message)
{
	/*	在这里，将经过若干步骤的运算	*/
	conn->send(message.data(), message.size());
}

/*	数据发送完成后，在TcpServer类中回调此函数			*/
void DetectServer::HandleSendComplete(spConnection conn)
{
	//printf("message send complete.\n");

	// 根据业务的需求，在这里可以增加其它的代码
}

/*	epoll_wait()超时，在TcpServer类中回调此函数			*/
void DetectServer::HandleTimeOut(EventLoop* loop)
{
	printf("epoll_wait() timeout.\n");

	// 根据业务的需求，在这里可以增加其它的代码
}

/*	Draw bounding box for image							*/
void DetectServer::Draw(cv::Mat& img,
	const std::vector<std::tuple<cv::Rect, float, int>>& data_vec,
	const std::vector<std::string>& class_names,
	bool label)
{
	for (const auto& data : data_vec) {
		cv::Rect box;
		float score;
		int class_idx;
		std::tie(box, score, class_idx) = data;

		cv::rectangle(img, box, cv::Scalar(0, 0, 255), 2);

		if (label) {
			std::stringstream ss;
			ss << std::fixed << std::setprecision(2) << score;
			std::string s = class_names[class_idx] + " " + ss.str();

			auto font_face = cv::FONT_HERSHEY_DUPLEX;
			auto font_scale = 1.0;
			int thickness = 1;
			int baseline = 0;
			auto s_size = cv::getTextSize(s, font_face, font_scale, thickness, &baseline);
			cv::rectangle(img,
				cv::Point(box.tl().x, box.tl().y - s_size.height - 5),
				cv::Point(box.tl().x + s_size.width, box.tl().y),
				cv::Scalar(0, 0, 255), -1);
			cv::putText(img, s, cv::Point(box.tl().x, box.tl().y - 5),
				font_face, font_scale, cv::Scalar(255, 255, 255), thickness);
		}
	}
}