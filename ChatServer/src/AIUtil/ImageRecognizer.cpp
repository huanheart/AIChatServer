#include "../include/AIUtil/ImageRecognizer.h"
#include <iostream>
#include <algorithm>

ImageRecognizer::ImageRecognizer(const std::string& model_path)
    : env(ORT_LOGGING_LEVEL_WARNING, "ImageRecognizer")
{
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    session = std::make_unique<Ort::Session>(env, model_path.c_str(), session_options);
    allocator = std::make_unique<Ort::AllocatorWithDefaultOptions>();

    // 获取输入输出名字
    input_name = session->GetInputNameAllocated(0, *allocator).get();
    output_name = session->GetOutputNameAllocated(0, *allocator).get();

    // 假设输入是 [1,3,H,W]
    input_shape = session->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    input_height = static_cast<int>(input_shape[2]);
    input_width = static_cast<int>(input_shape[3]);
}

int ImageRecognizer::PredictFromFile(const std::string& image_path) {
    cv::Mat img = cv::imread(image_path);
    if (img.empty()) {
        std::cerr << "Failed to load image: " << image_path << std::endl;
        return -1;
    }
    return PredictFromMat(img);
}

int ImageRecognizer::PredictFromBuffer(const std::vector<unsigned char>& image_data) {
    cv::Mat img = cv::imdecode(image_data, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Failed to decode image from buffer" << std::endl;
        return -1;
    }
    return PredictFromMat(img);
}

int ImageRecognizer::PredictFromMat(const cv::Mat& img_raw) {
    if (img_raw.empty()) return -1;

    cv::Mat img;
    cv::resize(img_raw, img, cv::Size(input_width, input_height));
    img.convertTo(img, CV_32F, 1.0 / 255.0);

    // NHWC -> NCHW
    cv::dnn::blobFromImage(img, img);

    std::vector<int64_t> dims = { 1, 3, input_height, input_width };
    size_t input_tensor_size = 1 * 3 * input_height * input_width;

    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
        OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info, img.ptr<float>(), input_tensor_size, dims.data(), dims.size());

    // ONNX Runtime 1.20 正确调用方式
    const char* input_names[] = { input_name.c_str() };
    const char* output_names[] = { output_name.c_str() };

    auto output_tensors = session->Run(
        Ort::RunOptions{ nullptr },
        input_names, &input_tensor, 1,
        output_names, 1
    );

    float* output_data = output_tensors.front().GetTensorMutableData<float>();

    // 获取最大概率的分类（假设是1000类 ImageNet）
    int pred_class = std::max_element(output_data, output_data + 1000) - output_data;
    return pred_class;
}
