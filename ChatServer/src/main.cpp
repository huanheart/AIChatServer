#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include"../include/ChatServer.h"

const std::string RABBITMQ_HOST = "localhost";
const std::string QUEUE_NAME = "sql_queue";
const int THREAD_NUM = 2;

void executeMysql(const std::string sql) {
    http::MysqlUtil mysqlUtil_;
    mysqlUtil_.executeUpdate(sql);
}


int main(int argc, char* argv[]) {
	LOG_INFO << "pid = " << getpid();
	std::string serverName = "ChatServer";
	int port = 80;
    // ��������
    int opt;
    const char* str = "p:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            port = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
    muduo::Logger::setLogLevel(muduo::Logger::WARN);
    ChatServer server(port, serverName);
    server.setThreadNum(4);
    //���һ��Ҫ����˯�߲������������ChatServer���캯���г�ʼ����ֿ�������
    std::this_thread::sleep_for(std::chrono::seconds(2));
    //��ʼ��chat_message��chatInformation��
    server.initChatMessage();    

    // ��ʼ�����Ѷ��е��̳߳أ����봦��������������̶߳���ͳһ�Ĵ������߼���
    //���Ҫ����Э�̿�����ÿ���߳�����ͬ��������ôҲ�����ٷ�һ�������࣬�߳���ȡ�������еĺ�������ִ�в���
    RabbitMQThreadPool pool(RABBITMQ_HOST, QUEUE_NAME, THREAD_NUM, executeMysql);
    pool.start();

    server.start();
}
