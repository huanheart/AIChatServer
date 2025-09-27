#include "../include/handlers/ChatHistoryHandler.h"

void ChatHistoryHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    try
    {
        // ����û��Ƿ��ѵ�¼
        auto session = server_->getSessionManager()->getSession(req, resp);
        LOG_INFO << "session->getValue(\"isLoggedIn\") = " << session->getValue("isLoggedIn");
        if (session->getValue("isLoggedIn") != "true")
        {
            // �û�δ��¼������δ��Ȩ����
            json errorResp;
            errorResp["status"] = "error";
            errorResp["message"] = "Unauthorized";
            std::string errorBody = errorResp.dump(4);

            server_->packageResp(req.getVersion(), http::HttpResponse::k401Unauthorized,
                "Unauthorized", true, "application/json", errorBody.size(),
                errorBody, resp);
            return;
        }

        // ��ȡ�û���Ϣ�Լ���ȡ�û���Ӧ�ı�����
        int userId = std::stoi(session->getValue("userId"));
        std::string username = session->getValue("username");
        std::vector<std::pair<std::string, long long>> messages;
        {
            std::shared_ptr<AIHelper> AIHelperPtr;
            std::lock_guard<std::mutex> lock(server_->mutexForChatInformation);
            if (server_->chatInformation.find(userId) == server_->chatInformation.end()) {
                // ����һ���µ� AIHelper
                server_->chatInformation.emplace(
                    userId,
                    std::make_shared<AIHelper>()
                );
            }
            AIHelperPtr = server_->chatInformation[userId];
            messages= AIHelperPtr->GetMessages();
        }
        //start
        json successResp;
        successResp["success"] = true;
        successResp["history"] = json::array();

        for (size_t i = 0; i < messages.size(); ++i) {
            json msgJson;
            msgJson["is_user"] = (i % 2 == 0);
            msgJson["content"] = messages[i].first;
            successResp["history"].push_back(msgJson);
        }

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
        // �����쳣�����ش�����Ϣ
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









