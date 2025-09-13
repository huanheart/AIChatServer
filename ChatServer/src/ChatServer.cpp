#include "../include/handlers/ChatLoginHandler.h"
#include "../include/handlers/ChatRegisterHandler.h"
#include "../include/handlers/ChatLogoutHandler.h"
#include"../include/handlers/ChatHandler.h"
#include"../include/handlers/ChatEntryHandler.h"
#include"../include/handlers/ChatSendHandler.h"
#include"../include/handlers/AIMenuHandler.h"
#include"../include/handlers/AIUploadSendHandler.h"
#include"../include/handlers/AIUploadHandler.h"
#include"../include/handlers/AIExitLoginHanndler.h"


#include "../include/ChatServer.h"
#include "../../../HttpServer/include/http/HttpRequest.h"
#include "../../../HttpServer/include/http/HttpResponse.h"
#include "../../../HttpServer/include/http/HttpServer.h"



using namespace http;


ChatServer::ChatServer(int port,
    const std::string& name,
    muduo::net::TcpServer::Option option)
    : httpServer_(port, name, option)
{
    initialize();
}

void ChatServer::initialize() {
	http::MysqlUtil::init("tcp://127.0.0.1:3306", "root", "123456", "ChatHttpServer", 10);
    // 初始化会话
    initializeSession();
    // 初始化中间件
    initializeMiddleware();
    // 初始化路由
    initializeRouter();
    //初始化chat_message表到chatInformation中
    initChatMessage();
}

void ChatServer::initChatMessage() {
    //遍历chat_message所有数据，将指定user_id对应消息放入到chatInformation中
    readDataFromMySQL();
}

void ChatServer::readDataFromMySQL() {
    //按照时间戳排序
    std::string sql = "SELECT id, username, is_user, content, ts FROM chat_message ORDER BY ts ASC, id ASC";
    sql::ResultSet* res = mysqlUtil_.executeQuery(sql);

    while (res->next()) {
        std::string username = res->getString("username");
        /*
            这里不太需要根据is_user进行插入, 默认在聊天的时候就是一问一答的，默认问的时间永远在答的前面
            且默认一定是有偶数个消息在message中，暂且不做太多健壮性考虑
            bool is_user = res->getInt("is_user") != 0; 
        */
        std::string content = res->getString("content");
        long long ts = res->getInt64("ts");
        long long user_id = res->getInt64("id");

        // 找到或创建对应的 AIHelper
        std::shared_ptr<AIHelper> helper;
        auto it = chatInformation.find(user_id);
        if (it == chatInformation.end()) {
            helper = std::make_shared<AIHelper>();
            chatInformation[user_id] = helper;
        }else {
            helper = it->second;
        }
        // 添加消息
        helper->addMessage(content,ts);
    }
}


void ChatServer::setThreadNum(int numThreads) {
    httpServer_.setThreadNum(numThreads);
}


void ChatServer::start() {
    httpServer_.start();
}


void ChatServer::initializeRouter() {
    // 注册url回调处理器
    // 登录注册入口页面
    httpServer_.Get("/", std::make_shared<ChatEntryHandler>(this));
    httpServer_.Get("/entry", std::make_shared<ChatEntryHandler>(this));
    // 登录
    httpServer_.Post("/login", std::make_shared<ChatLoginHandler>(this));
    // 注册
    httpServer_.Post("/register", std::make_shared<ChatRegisterHandler>(this));
    //登出
    httpServer_.Post("/user/logout", std::make_shared<ChatLogoutHandler>(this));
    //聊天页面入口
    httpServer_.Get("/chat", std::make_shared<ChatHandler>(this));
    //聊天请求
    httpServer_.Post("/chat/send", std::make_shared<ChatSendHandler>(this));
    //菜单页面
    httpServer_.Get("/menu", std::make_shared<AIMenuHandler>(this));
    //上传页面入口
    httpServer_.Get("/upload", std::make_shared<AIUploadHandler>(this));
    //上传请求
    httpServer_.Post("/upload/send", std::make_shared<AIUploadSendHandler>(this));

}

void ChatServer::initializeSession() {
    // 创建会话存储
    auto sessionStorage = std::make_unique<http::session::MemorySessionStorage>();
    // 创建会话管理器
    auto sessionManager = std::make_unique<http::session::SessionManager>(std::move(sessionStorage));
    // 设置会话管理器
    setSessionManager(std::move(sessionManager));
}

void ChatServer::initializeMiddleware() {
    // 创建中间件
    auto corsMiddleware = std::make_shared<http::middleware::CorsMiddleware>();
    // 添加中间件
    httpServer_.addMiddleware(corsMiddleware);
}


void ChatServer::packageResp(const std::string& version,
    http::HttpResponse::HttpStatusCode statusCode,
    const std::string& statusMsg,
    bool close,
    const std::string& contentType,
    int contentLen,
    const std::string& body,
    http::HttpResponse* resp)
{
    if (resp == nullptr)
    {
        LOG_ERROR << "Response pointer is null";
        return;
    }

    try
    {
        resp->setVersion(version);
        resp->setStatusCode(statusCode);
        resp->setStatusMessage(statusMsg);
        resp->setCloseConnection(close);
        resp->setContentType(contentType);
        resp->setContentLength(contentLen);
        resp->setBody(body);

        LOG_INFO << "Response packaged successfully";
    }
    catch (const std::exception& e)
    {
        LOG_ERROR << "Error in packageResp: " << e.what();
        // 设置一个基本的错误响应
        resp->setStatusCode(http::HttpResponse::k500InternalServerError);
        resp->setStatusMessage("Internal Server Error");
        resp->setCloseConnection(true);
    }
}
