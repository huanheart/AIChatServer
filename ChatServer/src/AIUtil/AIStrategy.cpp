#include"../include/AIUtil/AIStrategy.h"
#include"AIFactory.h"

std::string AliyunStrategy::getApiUrl()  {
    return "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions";
}

std::string AliyunStrategy::getApiKey() {
    return apiKey_;
}


std::string AliyunStrategy::getModel() {
    return "qwen-plus";
}


std::string DouBaoStrategy::getApiUrl() {
    return "https://ark.cn-beijing.volces.com/api/v3/chat/completions";
}

std::string DouBaoStrategy::getApiKey() {
    return apiKey_;
}


std::string DouBaoStrategy::getModel() {
    return "doubao-seed-1-6-thinking-250715";
}

// ×¢²áµ½¹¤³§
static StrategyRegister<AliyunStrategy> regAliyun("1");
static StrategyRegister<DouBaoStrategy> regDoubao("2");
