#pragma once

#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <vector>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <iostream>
#include <chrono>


class MQManager {
public:
    static MQManager& instance() {
        static MQManager mgr;
        return mgr;
    }

    void publish(const std::string& queue, const std::string& msg);

private:
    struct MQConn {
        AmqpClient::Channel::ptr_t channel;
        std::mutex mtx;
    };

    MQManager(size_t poolSize = 5) : poolSize_(poolSize), counter_(0) {
        for (size_t i = 0; i < poolSize_; ++i) {
            auto conn = std::make_shared<MQConn>();
            conn->channel = AmqpClient::Channel::Create("localhost");
            conn->channel->DeclareQueue("sql_queue", false, true, false, false);
            pool_.push_back(conn);
        }
    }

    MQManager(const MQManager&) = delete;
    MQManager& operator=(const MQManager&) = delete;

    std::vector<std::shared_ptr<MQConn>> pool_;
    size_t poolSize_;
    std::atomic<size_t> counter_;
};


class RabbitMQThreadPool {
private:
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_;
    std::string queue_name_;
    int thread_num_;
    std::string rabbitmq_host_;
    void worker(int id);

public:
    using HandlerFunc = std::function<void(const std::string&)>;

    RabbitMQThreadPool(const std::string& host,const std::string& queue,int thread_num,HandlerFunc handler)
                        : stop_(false), rabbitmq_host_(host), queue_name_(queue),
                        thread_num_(thread_num), handler_(handler) {}

    void start();
    void shutdown(); 

    ~RabbitMQThreadPool() {
        shutdown();
    }

};
