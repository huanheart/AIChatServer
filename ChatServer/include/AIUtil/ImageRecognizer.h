#pragma once
#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>
#include <string>
#include <vector>
#include <memory>

class ImageRecognizer {
public:
    // 构造函数，传入模型路径
    explicit ImageRecognizer(const std::string& model_path);

    // 从文件路径预测
    int PredictFromFile(const std::string& image_path);

    // 从内存数据预测（前端上传的图片二进制流）
    int PredictFromBuffer(const std::vector<unsigned char>& image_data);

    // 从 OpenCV Mat 预测
    int PredictFromMat(const cv::Mat& img);

private:
    Ort::Env env;
    std::unique_ptr<Ort::Session> session;
    std::unique_ptr<Ort::AllocatorWithDefaultOptions> allocator;

    std::string input_name;
    std::string output_name;
    std::vector<int64_t> input_shape;
    int input_height{}, input_width{};
};

#endif // IMAGE_RECOGNIZER_H
