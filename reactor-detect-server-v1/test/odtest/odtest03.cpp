/*
* 
* 测试目标检测服务器的并发性能
* 
* 
* 
*/


/*
./odtest03 www.hexinghuacloud.com 5001&
*/

#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <fstream>
#include <time.h>


int main(int argc, const char* argv[])
{
	if (argc != 3)
	{
		printf("usage: ./odclient ip port\n");
		printf("example: ./odclient www.hexinghuacloud.com 5001\n\n");
		return -1;
	}

	// load image.
	cv::Mat img;
	try
	{
		img = cv::imread("/root/xinghua.he/yolov7-deploy/weights/horses.jpg");
		if (img.empty())
		{
			std::cout << "Empty loading the iamge!" << std::endl;
			return -1;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error loading the image!\n";
		return -1;
	}

	cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
	cv::Mat img_tmp = img - 128;
	std::vector<uchar> data_encode;
	cv::imencode(".jpg", img_tmp, data_encode);
	int len_encode = data_encode.size();
	std::string buf;
	buf.append((char*)&len_encode, 4);
	buf.append((char*)data_encode.data(), len_encode);

	// socket.
	int servsock;
	if ((servsock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket() failed.\n");
		return -1;
	}

	// socket address.
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	struct hostent* h;
	if ((h = gethostbyname(argv[1])) == nullptr)
	{
		printf("gethostbyname failed.\n");
		close(servsock);
		return -1;
	}
	memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);

	// connect.
	if (connect(servsock, (sockaddr*)&servaddr, sizeof(servaddr)) != 0)
	{
		printf("connect(%s:%s) failed.\n", argv[1], argv[2]);
		close(servsock);
		return -1;
	}

	printf("Start time: %ld\n", time(0));


	// send.
	send(servsock, buf.c_str(), len_encode + 4, 0);
	// receive.
	char* tmpbuf = new char[4 + len_encode];
	int len;
	recv(servsock, &len, 4, 0); // 先读取4字节的报文头部
	memset(tmpbuf, 0, sizeof(tmpbuf));
	recv(servsock, tmpbuf, len_encode, 0); // 读取报文内容

	delete[] tmpbuf;
	close(servsock);

	printf("End time:%ld\n", time(0));

	return 0;
}