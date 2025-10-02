#include "../include/handlers/ChatLoginHandler.h"

void ChatLoginHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    // ¼߼
    // ֤ contentType
    auto contentType = req.getHeader("Content-Type");
    if (contentType.empty() || contentType != "application/json" || req.getBody().empty())
    {
        LOG_INFO << "content" << req.getBody();
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
        json parsed = json::parse(req.getBody());
        std::string username = parsed["username"];
        std::string password = parsed["password"];
        // ֤ûǷ
        int userId = queryUserId(username, password);
        if (userId != -1)
        {
            // ȡỰ
            auto session = server_->getSessionManager()->getSession(req, resp);
            // ỰͬһỰΪỰжǲͬһỰͨеcookieжϵ
            // ԲͬҳķǲͬĻỰģֻиҳǰʹˣŻлỰ¼
            // ôжûǷط¼вͨỰж

            // ڻỰд洢ûϢ
            session->setValue("userId", std::to_string(userId));
            session->setValue("username", username);
            session->setValue("isLoggedIn", "true");
            if (server_->onlineUsers_.find(userId) == server_->onlineUsers_.end() || server_->onlineUsers_[userId] == false)
            {
                {
                    std::lock_guard<std::mutex> lock(server_->mutexForOnlineUsers_);
                    server_->onlineUsers_[userId] = true;
                }

                // ûڵ¼ɹ
                // װjson ݡ
                json successResp;
                successResp["success"] = true;
                successResp["userId"] = userId;
                std::string successBody = successResp.dump(4);

                resp->setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
                resp->setCloseConnection(false);
                resp->setContentType("application/json");
                resp->setContentLength(successBody.size());
                resp->setBody(successBody);
                return;
            }
            else
            {
                // FIXME: ǰûط¼Уԭе¼ûǿ߸
                // ظ¼
                json failureResp;
                failureResp["success"] = false;
                failureResp["error"] = "˺ط¼";
                std::string failureBody = failureResp.dump(4);

                resp->setStatusLine(req.getVersion(), http::HttpResponse::k403Forbidden, "Forbidden");
                resp->setCloseConnection(true);
                resp->setContentType("application/json");
                resp->setContentLength(failureBody.size());
                resp->setBody(failureBody);
                return;
            }
        }
        else // ˺µ¼
        {
            // װjson
            json failureResp;
            failureResp["status"] = "error";
            failureResp["message"] = "Invalid username or password";
            std::string failureBody = failureResp.dump(4);

            resp->setStatusLine(req.getVersion(), http::HttpResponse::k401Unauthorized, "Unauthorized");
            resp->setCloseConnection(false);
            resp->setContentType("application/json");
            resp->setContentLength(failureBody.size());
            resp->setBody(failureBody);
            return;
        }
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
        return;
    }

}

int ChatLoginHandler::queryUserId(const std::string& username, const std::string& password)
{
    // ǰû˺룬ݿǷи˺
    // ʹԤ, ֹsqlע
    std::string sql = "SELECT id FROM users WHERE username = ? AND password = ?";
    // std::vector<std::string> params = {username, password};
    auto res = mysqlUtil_.executeQuery(sql, username, password);
    if (res->next())
    {
        int id = res->getInt("id");
        return id;
    }
    // ѯΪգ򷵻-1
    return -1;
}

