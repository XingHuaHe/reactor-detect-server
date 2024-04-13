/*
* 
* send a image to detectserver for test.
* 
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


void Draw(cv::Mat& img,
	const std::vector<std::tuple<cv::Rect, float, int>>& data_vec,
	const std::vector<std::string>& class_names,
	bool label);
std::vector<std::string> split(const std::string& s, char delimiter);
std::vector<std::string> GetClassNames(const std::string& path);

int main(int argc, const char* argv[])
{
	if (argc != 3)
	{
		printf("usage: ./odclient ip port\n");
		printf("example: ./odclient www.hexinghuacloud.com 5001\n\n");
		//return -1;
		argv[1] = "www.hexinghuacloud.com";
		argv[2] = "5001";
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

	//cv::Mat img_tmp = img - 128;
	//std::string buf;
	//int height = img.size().height;
	//int width = img.size().width;
	//int channels = img.channels();
	//int len = height * width * channels; // 1187328
	//buf.append((char*)&len, 4);

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
	printf("connect success.\n");

	// send.
	//std::cout << 4 + len_encode << std::endl;
	send(servsock, buf.c_str(), len_encode + 4, 0);
	
	// receive.
	char* tmpbuf = new char[4 + len_encode];
	int len;
	recv(servsock, &len, 4, 0);					// 先读取4字节的报文头部
	std::cout << len << std::endl;
	memset(tmpbuf, 0, sizeof(tmpbuf));
	recv(servsock, tmpbuf, len_encode, 0);	// 读取报文内容。
	//printf("recv:%s\n", tmpbuf);

	// show.
	std::vector<std::tuple<cv::Rect, float, int>> detections;
	std::vector<std::string> results = split(tmpbuf, '\n');
	for (auto it : results)
	{
		std::vector<std::string> result = split(it, ','); // x,y,w,h,score,class,class_name
		
		if (result.empty()) continue;
		cv::Rect box(atoi(result[0].c_str()), atoi(result[1].c_str()), atoi(result[2].c_str()), atoi(result[3].c_str()));
		float score = atof(result[4].c_str());
		int class_idx = atoi(result[5].c_str());
		std::tuple<cv::Rect, float, int> t = std::make_tuple(box, score, class_idx);

		detections.push_back(t);
	}

	Draw(img, detections, GetClassNames("/root/xinghua.he/reactor-detect-server/res/coco.names"), true);
	cv::imwrite("/root/xinghua.he/yolov7-deploy/result.jpg", img);


	delete[] tmpbuf;
	close(servsock);

	return 0;
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

void Draw(cv::Mat& img,
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

std::vector<std::string> GetClassNames(const std::string& path)
{
	std::vector<std::string> class_names;
	// load class names
	std::ifstream infile(path);

	if (infile.is_open())
	{
		std::string line;
		while (getline(infile, line))
		{
			class_names.emplace_back(line);
		}
		infile.close();
	}
	else
	{
		std::cerr << "Error loading the class names!\n";
	}

	return class_names;
}