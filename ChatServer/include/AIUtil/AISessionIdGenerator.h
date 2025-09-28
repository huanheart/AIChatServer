#pragma once
#include <chrono>
#include <random>
#include <cstdlib>
#include <ctime>
#include <string>


class AISessionIdGenerator {
public:
    AISessionIdGenerator() {
        // 初始化随机种子，只需一次
        std::srand(static_cast<unsigned>(std::time(nullptr)));
    }
    //利用时间戳生成一个id
    long long generate();
};
