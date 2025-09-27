#pragma once
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <sstream>
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>


#include"AIStrategy.h"

class StrategyFactory {

public:
    using Creator = std::function<std::shared_ptr<AIStrategy>()>;

    static StrategyFactory& instance();

    void registerStrategy(const std::string& name, Creator creator);

    std::shared_ptr<AIStrategy> create(const std::string& name);

private:
    StrategyFactory() = default;
    std::unordered_map<std::string, Creator> creators;
};



//注意：这里可以设计为static std::shared_ptr<AIStrategy> instance = std::make_shared<T>();
//即后续多个用户是共享一个"策略"的，不会每次在map中查到了后新创建一个策略

//但是我在代码中依旧没写static，保证每个用户都有自己独特的"策略"，保证线程安全

template<typename T>
struct StrategyRegister {
    StrategyRegister(const std::string& name) {
        StrategyFactory::instance().registerStrategy(name, [] {
            std::shared_ptr<AIStrategy> instance = std::make_shared<T>();
            return instance;
            });
    }
};

