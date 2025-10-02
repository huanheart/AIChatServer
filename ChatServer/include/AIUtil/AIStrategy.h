#pragma once
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <sstream>
#include <memory>

#include "../../../../HttpServer/include/utils/JsonUtil.h"


// Խӿ
class AIStrategy {
public:
    virtual ~AIStrategy() = default;

    // APIַ
    virtual std::string getApiUrl() const = 0;

    // API Key
    virtual std::string getApiKey() const = 0;

    //ضӦmodel
    virtual std::string getModel() const = 0;

    // 
    virtual json buildRequest(const std::vector<std::pair<std::string, long long>>& messages) const = 0;

    // Ӧ
    virtual std::string parseResponse(const json& response) const = 0;

};

class AliyunStrategy : public AIStrategy {

public:
    AliyunStrategy() {
        const char* key = std::getenv("DASHSCOPE_API_KEY");
        if (!key) throw std::runtime_error("Aliyun API Key not found!");
        apiKey_ = key;
    }

    std::string getApiUrl() const override;
    std::string getApiKey() const override;
    std::string getModel() const override;

    json buildRequest(const std::vector<std::pair<std::string, long long>>& messages) const override;
    std::string parseResponse(const json& response) const override;

private:
    std::string apiKey_;
};

class DouBaoStrategy : public AIStrategy {

public:
    DouBaoStrategy() {
        const char* key = std::getenv("DOUBAO_API_KEY");
        if (!key) throw std::runtime_error("DOUBAO API Key not found!");
        apiKey_ = key;
    }
    std::string getApiUrl() const override;
    std::string getApiKey() const override;
    std::string getModel() const override;

    json buildRequest(const std::vector<std::pair<std::string, long long>>& messages) const override;
    std::string parseResponse(const json& response) const override;

private:
    std::string apiKey_;
};

class AliyunRAGStrategy : public AIStrategy {

public:
    AliyunRAGStrategy() {
        const char* key = std::getenv("DASHSCOPE_API_KEY");
        if (!key) throw std::runtime_error("Aliyun API Key not found!");
        apiKey_ = key;
    }

    std::string getApiUrl() const override;
    std::string getApiKey() const override;
    std::string getModel() const override;

    json buildRequest(const std::vector<std::pair<std::string, long long>>& messages) const override;
    std::string parseResponse(const json& response) const override;

private:
    std::string apiKey_;
};





