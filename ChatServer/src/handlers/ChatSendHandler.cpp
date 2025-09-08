#include "../include/handlers/ChatSendHandler.h"

void ChatSendHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    try
    {
        // 检查用户是否已登录
        auto session = server_->getSessionManager()->getSession(req, resp);
        LOG_INFO << "session->getValue(\"isLoggedIn\") = " << session->getValue("isLoggedIn");
        if (session->getValue("isLoggedIn") != "true")
        {
            // 用户未登录，返回未授权错误
            json errorResp;
            errorResp["status"] = "error";
            errorResp["message"] = "Unauthorized";
            std::string errorBody = errorResp.dump(4);

            server_->packageResp(req.getVersion(), http::HttpResponse::k401Unauthorized,
                "Unauthorized", true, "application/json", errorBody.size(),
                errorBody, resp);
            return;
        }

        // 获取用户信息以及获取用户对应的表数据
        int userId = std::stoi(session->getValue("userId"));
        std::string username = session->getValue("username");
        std::shared_ptr<AIHelper> AIHelperPtr;
        {
            std::lock_guard<std::mutex> lock(server_->mutexForChatInformation);
            if (server_->chatInformation.find(userId) == server_->chatInformation.end()) {
                //从linux环境变量中拿取对应的api-key并初始化一个AIHelper
                const char* apiKey = std::getenv("DASHSCOPE_API_KEY");
                if (!apiKey) {
                    std::cerr << "Error: DASHSCOPE_API_KEY not found in environment!" << std::endl;
                    return;
                }
                // 插入一个新的 AIHelper
                server_->chatInformation.emplace(
                    userId,           
                    std::make_shared<AIHelper>(apiKey)
                );
            }
            AIHelperPtr= server_->chatInformation[userId];
        }

        std::string userQuestion;
        auto body = req.getBody();
        if (!body.empty()) {
            auto j = json::parse(body);
            if (j.contains("question")) userQuestion = j["question"];
        }

        AIHelperPtr->addUserMessage(userQuestion);


        std::string aiInformation=AIHelperPtr->chat();
        json successResp;
        successResp["success"] = true;
        successResp["Information"] = aiInformation;
        std::string successBody = successResp.dump(4);

        resp->setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
        resp->setCloseConnection(false);
        resp->setContentType("application/json");
        resp->setContentLength(successBody.size());
        resp->setBody(successBody);
        return;
    }
    catch (const std::exception& e)
    {
        // 捕获异常，返回错误信息
        json failureResp;
        failureResp["status"] = "error";
        failureResp["message"] = e.what();
        std::string failureBody = failureResp.dump(4);
        resp->setStatusLine(req.getVersion(), http::HttpResponse::k400BadRequest, "Bad Request");
        resp->setCloseConnection(true);
        resp->setContentType("application/json");
        resp->setContentLength(failureBody.size());
        resp->setBody(failureBody);
    }
}









