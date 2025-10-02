#include "../include/handlers/ChatSendHandler.h"


void ChatSendHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    try
    {
        // ûǷѵ¼
        auto session = server_->getSessionManager()->getSession(req, resp);
        LOG_INFO << "session->getValue(\"isLoggedIn\") = " << session->getValue("isLoggedIn");
        if (session->getValue("isLoggedIn") != "true")
        {
            // ûδ¼δȨ
            json errorResp;
            errorResp["status"] = "error";
            errorResp["message"] = "Unauthorized";
            std::string errorBody = errorResp.dump(4);

            server_->packageResp(req.getVersion(), http::HttpResponse::k401Unauthorized,
                "Unauthorized", true, "application/json", errorBody.size(),
                errorBody, resp);
            return;
        }

        // ȡûϢԼȡûӦı
        int userId = std::stoi(session->getValue("userId"));
        std::string username = session->getValue("username");

        std::string userQuestion;
        std::string modelType;
        std::string sessionId;

        auto body = req.getBody();
        if (!body.empty()) {
            auto j = json::parse(body);
            if (j.contains("question")) userQuestion = j["question"];
            if (j.contains("sessionId")) sessionId = j["sessionId"];
            // Ĭϰ
            modelType = j.contains("modelType") ? j["modelType"].get<std::string>() : "1";
        }


        std::shared_ptr<AIHelper> AIHelperPtr;
        {
            std::lock_guard<std::mutex> lock(server_->mutexForChatInformation);

            auto& userSessions = server_->chatInformation[userId];

            if (userSessions.find(sessionId) == userSessions.end()) {
                // һµ AIHelper
                userSessions.emplace( 
                    sessionId,
                    std::make_shared<AIHelper>()
                );
            }
            AIHelperPtr= userSessions[sessionId];
        }
        
        //óûҪĲ
        AIHelperPtr->setStrategy(StrategyFactory::instance().create(modelType));


        //int userId, const std::string& userName, bool is_user, const std::string& userInput,std::string sessionId;
        AIHelperPtr->addMessage(userId, username,true,userQuestion,sessionId);

        std::string aiInformation=AIHelperPtr->chat(userId, username,sessionId);
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
        // 쳣شϢ
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









