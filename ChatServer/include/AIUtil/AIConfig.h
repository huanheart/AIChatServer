#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <regex>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../../../../HttpServer/include/utils/JsonUtil.h"  // �����װ�� nlohmann::json

// �ṹ�壺����������Ϣ
struct AITool {
    std::string name;
    std::unordered_map<std::string, std::string> params;
    std::string desc;
};

// �ṹ�壺AI ��Ӧ�й��ߵ��ý��
struct AIToolCall {
    std::string toolName;
    json args;
    bool isToolCall = false;
};

// ���ù�����
class AIConfig {
public:
    bool loadFromFile(const std::string& path);
    std::string buildPrompt(const std::string& userInput) const;
    AIToolCall parseAIResponse(const std::string& response) const;
    std::string buildToolResultPrompt(const std::string& userInput,const std::string& toolName,const json& toolArgs,const json& toolResult) const;

private:
    std::string promptTemplate_;
    std::vector<AITool> tools_;

    std::string buildToolList() const;
};
