/*
* 
* 测试 detector.h/cpp 的 Detector 功能
* 测试 yolov7-tiny 部署.
* 
* 
*/

#include <torch/script.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "detector.h"


void Demo(cv::Mat& img,
	const std::vector<std::tuple<cv::Rect, float, int>>& data_vec,
	const std::vector<std::string>& class_names,
	bool label = true) {
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

	//cv::namedWindow("Result", cv::WINDOW_AUTOSIZE);
	cv::imwrite("/root/xinghua.he/yolov7-deploy/weights/result.jpg", img);
	//cv::imshow("Result", img);
	//cv::waitKey(0);
}


int main(int argc, const char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "usage: example <path-to-exported-script-module> <path-image>\n";
		std::cerr << "usage: odtest /root/xinghua.he/yolov7-deploy/weights/yolov7-tiny.torchscript.pt /root/xinghua.he/yolov7-deploy/weights/horses.jpg\n";
		return -1;
	}

	// 1. define model.
	Detector detector(argv[1]);
	// Load class names.
	detector.LoadClassNames("/root/xinghua.he/reactor-detect-server/res/coco.names");

	// 3. Load image.
	cv::Mat img;
	try
	{
		img = cv::imread(argv[2]);
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

	// 4. Inference.
	std::vector<std::tuple<cv::Rect, float, int>> results = detector.Forward(img);
	
	// 5. Save.
	Demo(img, results, detector.GetClassNames());

	return 0;
}