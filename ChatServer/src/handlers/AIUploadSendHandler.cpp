#include "../include/handlers/AIUploadSendHandler.h"


void AIUploadSendHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    try
    {
        // 检查用户是否已登录
        auto session = server_->getSessionManager()->getSession(req, resp);
        LOG_INFO << "session->getValue(\"isLoggedIn\") = " << session->getValue("isLoggedIn");
        if (session->getValue("isLoggedIn") != "true")
        {
            // 用户未登录，返回未授权错误
            json errorResp;
            errorResp["status"] = "error";
            errorResp["message"] = "Unauthorized";
            std::string errorBody = errorResp.dump(4);

            server_->packageResp(req.getVersion(), http::HttpResponse::k401Unauthorized,
                "Unauthorized", true, "application/json", errorBody.size(),
                errorBody, resp);
            return;
        }
        //处理对应流程start
        // 1. 解析 JSON 请求体
        int userId = std::stoi(session->getValue("userId"));
        std::shared_ptr<ImageRecognizer> ImageRecognizerPtr;
        {
            std::lock_guard<std::mutex> lock(server_->mutexForImageRecognizerMap);
            if (server_->ImageRecognizerMap.find(userId) == server_->ImageRecognizerMap.end()) {
                // 插入一个新的 ImageRecognizer
                server_->ImageRecognizerMap.emplace(
                    userId,
                    std::make_shared<ImageRecognizer>("/root/models/mobilenetv2/mobilenetv2-7.onnx")  //todo:需要将/path/to/model.onnx更改成真实路径
                );
            }
            ImageRecognizerPtr = server_->ImageRecognizerMap[userId];
        }

        auto body = req.getBody();
        std::string filename;
        std::string imageBase64;
        if (!body.empty()) {
            auto j = json::parse(body);
            if (j.filename("filename")) filename = j["filename"];
            if (j.filename("image")) imageBase64 = j["image"];
        }
        if (imageBase64.empty())
        {
            throw std::runtime_error("No image data provided");
        }

        // 2. base64 解码
        std::string decodedData = base64_decode(imageBase64);
        std::vector<uchar> imgData(decodedData.begin(), decodedData.end());

        // 3. 调用识别逻辑
        // 假设你在服务启动时已经初始化了 ImageRecognizer
        // 比如：ImageRecognizer recognizer("/root/model.onnx");
        int predClass = ImageRecognizerPtr->PredictFromBuffer(imgData);

        // TODO: 这里你应该有个类别表，比如 ImageNet 的 label
        // 我先用简单字符串代替
        std::string className = "class_" + std::to_string(predClass);

        // 4. 构造响应
        json successResp;
        successResp["success"] = "ok";
        successResp["filename"] = filename;
        successResp["class_name"] = className;
        //模型对这个的置信度
        successResp["confidence"] = 0.95; // todo:这里写死了，后续你可以从模型里返回真实的

        //end
        std::string successBody = successResp.dump(4);

        resp->setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
        resp->setCloseConnection(false);
        resp->setContentType("application/json");
        resp->setContentLength(successBody.size());
        resp->setBody(successBody);
        return;

    }
    catch (const std::exception& e)
    {
        // 捕获异常，返回错误信息
        json failureResp;
        failureResp["status"] = "error";
        failureResp["message"] = e.what();
        std::string failureBody = failureResp.dump(4);
        resp->setStatusLine(req.getVersion(), http::HttpResponse::k400BadRequest, "Bad Request");
        resp->setCloseConnection(true);
        resp->setContentType("application/json");
        resp->setContentLength(failureBody.size());
        resp->setBody(failureBody);
    }
}

/*

int main() {
    ImageRecognizer recog("resnet50.onnx");

    // 1. 从文件预测
    int cls1 = recog.PredictFromFile("dog.jpg");
    std::cout << "PredictFromFile: " << cls1 << std::endl;

    // 2. 从内存数据预测（模拟前端上传）
    std::ifstream file("dog.jpg", std::ios::binary);
    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)), {});
    int cls2 = recog.PredictFromBuffer(buffer);
    std::cout << "PredictFromBuffer: " << cls2 << std::endl;

    return 0;
}

*/

