#include"../include/AIUtil/MQManager.h"

void MQManager::publish(const std::string& queue, const std::string& msg) {
    size_t index = counter_.fetch_add(1) % poolSize_;
    auto& conn = pool_[index];

    std::lock_guard<std::mutex> lock(conn->mtx);
    auto message = AmqpClient::BasicMessage::Create(msg);
    conn->channel->BasicPublish("", queue, message);
}


void RabbitMQThreadPool::worker(int id){
    try {
        auto channel = AmqpClient::Channel::Create(rabbitmq_host_);
        channel->DeclareQueue(queue_name_, false, true, false, false);
        channel->BasicQos(1); // 每个线程一次只处理一条消息
        std::string consumer_tag = channel->BasicConsume(queue_name_, "");

        while (!stop_) {
            AmqpClient::Envelope::ptr_t env;
            bool ok = channel->BasicConsumeMessage(consumer_tag, env, 500); // 500ms 超时
            if (ok && env) {
                std::string msg = env->Message()->Body();
                handler(msg);
                channel->BasicAck(env);
            }
        }

        // 停止时取消消费
        channel->BasicCancel(consumer_tag);
    }
    catch (const std::exception& e) {
        std::cerr << "Thread " << id << " exception: " << e.what() << std::endl;
    }
}

void RabbitMQThreadPool::start() {
    for (int i = 0; i < thread_num_; ++i) {
        workers_.emplace_back(&RabbitMQThreadPool::worker, this, i);
    }
}

void RabbitMQThreadPool::shutdown() {
    stop_ = true;
    for (auto& t : workers_) {
        if (t.joinable()) t.join();
    }
}