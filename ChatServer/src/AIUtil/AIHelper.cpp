#include"../include/AIUtil/AIHelper.h"
#include <stdexcept>

// 构造函数
AIHelper::AIHelper(const std::string& apiKey)
    : apiKey_(apiKey) {}

// 设置默认模型
void AIHelper::setModel(const std::string& modelName) {
    model_ = modelName;
}

// 添加一条用户消息
void AIHelper::addUserMessage(const std::string& userInput) {
    messages.push_back(userInput);
    // TODO: 在这里调用 pushMessageToMysql() 异步入库
    pushMessageToMysql();
}


// 发送聊天消息
std::string AIHelper::chat() {
    // 构造 payload
    json payload;
    payload["model"] = model_;
    json msgArray = json::array();

    for (size_t i = 0; i < messages.size(); ++i) {
        json msg;
        if (i % 2 == 0) { // 偶数下标：用户
            msg["role"] = "user";
            msg["content"] = messages[i];
        }
        else { // 奇数下标：AI
            msg["role"] = "assistant";
            msg["content"] = messages[i];
        }
        msgArray.push_back(msg);
    }

    payload["messages"] = msgArray;

    // 执行请求
    json response = executeCurl(payload);

    if (response.contains("choices") && !response["choices"].empty()) {
        std::string answer = response["choices"][0]["message"]["content"];
        messages.push_back(answer); // 保存 AI 回复
        // TODO: 可在这里调用 pushMessageToMysql()
        //pushMessageToMysql();
        return answer;
    }

    return "[Error] 无法解析响应";
}

// 发送自定义请求体
json AIHelper::request(const json& payload) {
    return executeCurl(payload);
}

// 内部方法：执行 curl 请求
json AIHelper::executeCurl(const json& payload) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize curl");
    }

    std::string readBuffer;
    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + apiKey_;

    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string payloadStr = payload.dump();

    curl_easy_setopt(curl, CURLOPT_URL, apiUrl_.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }

    try {
        return json::parse(readBuffer);
    }
    catch (...) {
        throw std::runtime_error("Failed to parse JSON response: " + readBuffer);
    }
}

// curl 回调函数，把返回的数据写到 string buffer
size_t AIHelper::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* buffer = static_cast<std::string*>(userp);
    buffer->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

void AIHelper::pushMessageToMysql() {

}
