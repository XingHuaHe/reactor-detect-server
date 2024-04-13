#pragma once
#include "tcpserver.h"
#include "threadpool.h"
#include "detector.h"
#include <mutex>


class DetectServer
{
public:
	DetectServer(const uint16_t port, const std::string weight, int subthreadnum = 5, int workthreadnum = 5);
	~DetectServer();

	/*	��������	*/
	void Start();
	/*	ֹͣ����	*/
	void Stop();

	/*	�����¿ͻ�������������TcpServer���лص��˺���		*/
	void HandleNewConnection(spConnection conn);
	/*	�رտͻ��˵����ӣ���TcpServer���лص��˺���			*/
	void HandleClose(spConnection conn);
	/*	�ͻ��˵����Ӵ�����TcpServer���лص��˺���			*/
	void HandleError(spConnection conn);
	/*	����ͻ��˵������ģ���TcpServer���лص��˺���		*/
	void HandleMessage(spConnection conn, std::string& message);
	/*	���ݷ�����ɺ���TcpServer���лص��˺���			*/
	void HandleSendComplete(spConnection conn);
	/*	epoll_wait()��ʱ����TcpServer���лص��˺���			*/
	void HandleTimeOut(EventLoop *loop);

	/*	����ͻ��˵������ģ�������Ӹ��̳߳�				*/
	void OnMessage(spConnection conn, std::string& message);

	/*	Draw bounding box for image							*/
	void Draw(cv::Mat& img, const std::vector<std::tuple<cv::Rect, float, int>>& data_vec, const std::vector<std::string>& class_names, bool label = true);

private:
	TcpServer _tcpserver;
	int _workthreadnum;			// �����̳߳صĴ�С
	ThreadPool _threadpool;		// �����̳߳أ���Ϊ EchoServer HandleMessage�������м��㣬�ʴ˹����߳��ڸ����У�

	Detector _detector;			// object detector.
	std::mutex _mutex;			// ���� �Ļ�����
};

