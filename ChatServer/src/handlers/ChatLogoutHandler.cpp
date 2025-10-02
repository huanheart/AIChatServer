#include "../include/handlers/ChatLogoutHandler.h"

void ChatLogoutHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    auto contentType = req.getHeader("Content-Type");
    if (contentType.empty() || contentType != "application/json" || req.getBody().empty())
    {
        resp->setStatusLine(req.getVersion(), http::HttpResponse::k400BadRequest, "Bad Request");
        resp->setCloseConnection(true);
        resp->setContentType("application/json");
        resp->setContentLength(0);
        resp->setBody("");
        return;
    }

    // JSON ʹ try catch 쳣
    try
    {
        // ȡỰ
        auto session = server_->getSessionManager()->getSession(req, resp);
        // ȡûid
        int userId = std::stoi(session->getValue("userId"));
        // Ự
        session->clear();
        // ٻỰ
        server_->getSessionManager()->destroySession(session->getId());

        json parsed = json::parse(req.getBody());

        {   // ͷԴ
            std::lock_guard<std::mutex> lock(server_->mutexForOnlineUsers_);
            server_->onlineUsers_.erase(userId);
        }

        // Ӧ
        json response;
        response["message"] = "logout successful";
        std::string responseBody = response.dump(4);
        resp->setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
        resp->setCloseConnection(true);
        resp->setContentType("application/json");
        resp->setContentLength(responseBody.size());
        resp->setBody(responseBody);
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