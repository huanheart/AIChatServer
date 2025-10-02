#include "../include/handlers/ChatLoginHandler.h"
#include "../include/handlers/ChatRegisterHandler.h"
#include "../include/handlers/ChatLogoutHandler.h"
#include"../include/handlers/ChatHandler.h"
#include"../include/handlers/ChatEntryHandler.h"
#include"../include/handlers/ChatSendHandler.h"
#include"../include/handlers/AIMenuHandler.h"
#include"../include/handlers/AIUploadSendHandler.h"
#include"../include/handlers/AIUploadHandler.h"
#include"../include/handlers/ChatHistoryHandler.h"


#include"../include/handlers/ChatCreateAndSendHandler.h"
#include"../include/handlers/ChatSessionsHandler.h"
#include"../include/handlers/ChatSpeechHandler.h"

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
    std::cout << "ChatServer initialize start  ! " << std::endl;
	http::MysqlUtil::init("tcp://127.0.0.1:3306", "root", "123456", "ChatHttpServer", 5);
    // ʼỰ
    initializeSession();
    // ʼм
    initializeMiddleware();
    // ʼ·
    initializeRouter();
}

void ChatServer::initChatMessage() {
    //chat_messageݣָuser_idӦϢ뵽chatInformation
    std::cout << "initChatMessage start ! " << std::endl;
    readDataFromMySQL();
    std::cout << "initChatMessage success ! " << std::endl;
}

void ChatServer::readDataFromMySQL() {

    // SQL ѯ
    std::string sql = "SELECT id, username,session_id, is_user, content, ts FROM chat_message ORDER BY ts ASC, id ASC";

    sql::ResultSet* res;
    try {
        res = mysqlUtil_.executeQuery(sql);
    }
    catch (const std::exception& e) {
        std::cerr << "MySQL query failed: " << e.what() << std::endl;
        return;
    }

    while (res->next()) {
        long long user_id = 0;
        std::string session_id ;   // ����ỰID
        std::string username, content;
        long long ts = 0;
        int is_user = 1;

        try {
            user_id    = res->getInt64("id");          // ûID
            session_id = res->getString("session_id");  // ỰID
            username   = res->getString("username");
            content    = res->getString("content");
            ts         = res->getInt64("ts");
            is_user    = res->getInt("is_user");
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to read row: " << e.what() << std::endl;
            continue; // 쳣
        }

        auto& userSessions = chatInformation[user_id];

        std::shared_ptr<AIHelper> helper;
        auto itSession = userSessions.find(session_id);
        if (itSession == userSessions.end()) {
            helper = std::make_shared<AIHelper>();
            userSessions[session_id] = helper;
        } else {
            helper = itSession->second;
        }
        
        // ¼ỰID,Ϊÿû¼ҳչʾлỰ
        sessionsIdsMap[user_id].push_back(session_id);

        // ָϢ
        helper->restoreMessage(content, ts);
    }

    std::cout << "readDataFromMySQL finished" << std::endl;
}



void ChatServer::setThreadNum(int numThreads) {
    httpServer_.setThreadNum(numThreads);
}


void ChatServer::start() {
    httpServer_.start();
}


void ChatServer::initializeRouter() {
    // עurlص
    // ¼עҳ
    httpServer_.Get("/", std::make_shared<ChatEntryHandler>(this));
    httpServer_.Get("/entry", std::make_shared<ChatEntryHandler>(this));
    // ¼
    httpServer_.Post("/login", std::make_shared<ChatLoginHandler>(this));
    // ע
    httpServer_.Post("/register", std::make_shared<ChatRegisterHandler>(this));
    //ǳ
    httpServer_.Post("/user/logout", std::make_shared<ChatLogoutHandler>(this));
    //ҳ
    httpServer_.Get("/chat", std::make_shared<ChatHandler>(this));
    //
    httpServer_.Post("/chat/send", std::make_shared<ChatSendHandler>(this));
    //˵ҳ
    httpServer_.Get("/menu", std::make_shared<AIMenuHandler>(this));
    //ϴҳ
    httpServer_.Get("/upload", std::make_shared<AIUploadHandler>(this));
    //ϴ
    httpServer_.Post("/upload/send", std::make_shared<AIUploadSendHandler>(this));
    //ͬʷݣһε¼ݷظǰȾ
    httpServer_.Post("/chat/history", std::make_shared<ChatHistoryHandler>(this));

    //ڶ׶
    httpServer_.Post("/chat/send-new-session", std::make_shared<ChatCreateAndSendHandler>(this));
    httpServer_.Get("/chat/sessions", std::make_shared<ChatSessionsHandler>(this));

    httpServer_.Post("/chat/tts", std::make_shared<ChatSpeechHandler>(this));
}

void ChatServer::initializeSession() {
    // Ự洢
    auto sessionStorage = std::make_unique<http::session::MemorySessionStorage>();
    // Ự
    auto sessionManager = std::make_unique<http::session::SessionManager>(std::move(sessionStorage));
    // ûỰ
    setSessionManager(std::move(sessionManager));
}

void ChatServer::initializeMiddleware() {
    // м
    auto corsMiddleware = std::make_shared<http::middleware::CorsMiddleware>();
    // м
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
        // һĴӦ
        resp->setStatusCode(http::HttpResponse::k500InternalServerError);
        resp->setStatusMessage("Internal Server Error");
        resp->setCloseConnection(true);
    }
}
