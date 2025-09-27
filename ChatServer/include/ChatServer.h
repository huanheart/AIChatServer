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
#include"AIUtil/ImageRecognizer.h"
#include"AIUtil/base64.h"
#include"AIUtil/MQManager.h"


class ChatLoginHandler;
class ChatRegisterHandler;
class ChatLogoutHandler;
class ChatHandler;
class ChatEntryHandler;
class ChatSendHandler;
class ChatHistoryHandler;

class AIMenuHandler;
class AIUploadHandler;
class AIUploadSendHandler;



class ChatServer {
public:
	ChatServer(int port,
		const std::string& name,
		muduo::net::TcpServer::Option option = muduo::net::TcpServer::kNoReusePort);

	void setThreadNum(int numThreads);
	void start();
	void initChatMessage();
private:
	friend class ChatLoginHandler;
	friend class ChatRegisterHandler;
	friend  ChatLogoutHandler;
	friend class ChatHandler;
	friend class ChatEntryHandler;
	friend class ChatSendHandler;
	friend class AIMenuHandler;
	friend class AIUploadHandler;
	friend class AIUploadSendHandler;
	friend class ChatHistoryHandler;

private:
	void initialize();
	void initializeSession();
	void initializeRouter();
	void initializeMiddleware();
	

	void readDataFromMySQL();

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
	//����ǰ�˵�httpServer
	http::HttpServer	httpServer_;
	//�����ݿ⽻��
	http::MysqlUtil		mysqlUtil_;
	//��֤һ���û�ֻ����ͬһ���ص��¼һ��
	std::unordered_map<int, bool>	onlineUsers_;
	std::mutex	mutexForOnlineUsers_;
	//ÿһ���˶��ж�Ӧ�ĶԻ� userid->AIHelper
	//ע�⣺���ָ������Ϊ������Ҫ��chatInformation[key]���и���
	//��ֱ�Ӵ�Ŷ��󣬲����ָ�룬��ô����unordered_map�����̰߳�ȫ�ģ���Ҫ��������
	//�����vector���в����������ô�û�A����AI�Ĳ����ͻ�����Ӱ�쵽�û�B
	std::unordered_map<int, std::shared_ptr<AIHelper>> chatInformation;
	std::mutex	mutexForChatInformation;

	std::unordered_map<int, std::shared_ptr<ImageRecognizer> > ImageRecognizerMap;
	std::mutex	mutexForImageRecognizerMap;
};

