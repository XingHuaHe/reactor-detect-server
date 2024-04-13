#pragma once
#include <torch/script.h>
#include <opencv2/opencv.hpp>


enum Det
{
	tl_x = 0,
	tl_y = 1,
	br_x = 2,
	br_y = 3,
	score = 4,
	class_idx = 5
};


class Detector
{
public:
	Detector(const std::string& weight, const torch::DeviceType& device = torch::kCPU, float conf_threshold = 0.4, float iou_threshold = 0.6);
	~Detector();

	/*	inference.			*/
	std::vector<std::tuple<cv::Rect, float, int>> Forward(const cv::Mat& img);
	/*	load class names	*/
	void LoadClassNames(const std::string& path);
	/*	get class names		*/
	std::vector<std::string> GetClassNames();
	/*	loacal video stream.	*/
	std::string gstreamer_pipeline(int capture_width, int capture_height, int display_width, int display_height, int framerate, int flip_method);


private:
	const torch::Device _device;			// device type (cpu or gpu).
	torch::jit::script::Module _module;		// network model.
	float _conf_threshold;					//
	float _iou_threshold;					//
	std::vector<std::string> _class_names;	// class names.

	/*	compute bounding box iou.	*/
	static inline torch::Tensor GetBoundingBoxIoU(const torch::Tensor& box1, const torch::Tensor& box2);
	/*	reshape and scale image.	*/
	static std::vector<float> LetterboxImage(const cv::Mat& src, cv::Mat& dst, const cv::Size& out_size = cv::Size(640, 640));
	/*	results detected process.	*/
	static torch::Tensor PostProcessing(const torch::Tensor& detections, float conf_threshold = 0.4, float iou_threshold = 0.6);
};

