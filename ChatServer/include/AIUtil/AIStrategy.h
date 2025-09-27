#pragma once
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <sstream>
#include <memory>

// 策略接口
class AIStrategy {
public:
    virtual ~AIStrategy() = default;

    // 返回API地址
    virtual std::string getApiUrl() const = 0;

    // 返回API Key
    virtual std::string getApiKey() const = 0;

    //返回对应model
    virtual std::string getModel() const = 0;

    

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


private:
    std::string apiKey_;
};



