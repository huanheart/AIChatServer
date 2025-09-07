#pragma once

#include <atomic>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <mutex>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

#include "../../../HttpServer/include/http/HttpServer.h"
#include "../../../HttpServer/include/utils/MysqlUtil.h"
#include "../../../HttpServer/include/utils/FileUtil.h"
#include "../../../HttpServer/include/utils/JsonUtil.h"
#include"AIUtil/AIHelper.h"

class ChatLoginHandler;
class ChatRegisterHandler;
class ChatLogoutHandler;
class ChatHandler;

class ChatServer {
public:
	ChatServer(int port,
		const std::string& name,
		muduo::net::TcpServer::Option option);

	void setThreadNum(int numThreads);
	void start();

private:
	friend class ChatLoginHandler;
	friend class ChatRegisterHandler;
	friend class ChatLogoutHandler;
	friend class ChatHandler;

private:
	void initialize();
	void initializeSession();
	void initializeRouter();
	void initializeMiddleware();


	void packageResp(const std::string& version, http::HttpResponse::HttpStatusCode statusCode,
		const std::string& statusMsg, bool close, const std::string& contentType,
		int contentLen, const std::string& body, http::HttpResponse* resp);

	void setSessionManager(std::unique_ptr<http::session::SessionManager> manager)
	{
		httpServer_.setSessionManager(std::move(manager));
	}
	http::session::SessionManager* getSessionManager() const
	{
		return httpServer_.getSessionManager();
	}
	//监听前端的httpServer
	http::HttpServer	httpServer_;
	//和数据库交互
	http::MysqlUtil		mysqlUtil_;
	//保证一个用户只能在同一个地点登录一次
	std::unordered_map<int, bool>	onlineUsers_;
	std::mutex	mutexForOnlineUsers_;
	//每一个人都有对应的对话 userid->AIHelper
	std::unordered_map<int, std::shared_ptr<AIHelper>> chatInformation;
	std::mutex	mutexForChatInformation;
};

