#pragma once
#include <chrono>
#include <random>
#include <cstdlib>
#include <ctime>
#include <string>


class AISessionIdGenerator {
public:
    AISessionIdGenerator() {
        // ��ʼ��������ӣ�ֻ��һ��
        std::srand(static_cast<unsigned>(std::time(nullptr)));
    }
    //����ʱ�������һ��id
    long long generate();
};
