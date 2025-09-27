#include "../include/handlers/ChatLoginHandler.h"

void ChatLoginHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    // �����¼�߼�
    // ��֤ contentType
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

    // JSON ����ʹ�� try catch �����쳣
    try
    {
        json parsed = json::parse(req.getBody());
        std::string username = parsed["username"];
        std::string password = parsed["password"];
        // ��֤�û��Ƿ����
        int userId = queryUserId(username, password);
        if (userId != -1)
        {
            // ��ȡ�Ự
            auto session = server_->getSessionManager()->getSession(req, resp);
            // �Ự������ͬһ���Ự����Ϊ�Ự�ж��ǲ���ͬһ���Ự��ͨ���������е�cookie���жϵ�
            // ���Բ�ͬҳ��ķ����ǲ���������ͬ�ĻỰ�ģ�ֻ�и�ҳ��ǰ����ʹ�����ˣ��Ż��лỰ��¼
            // ��ô�ж��û��Ƿ��������ط���¼�в���ͨ���Ự���ж�

            // �ڻỰ�д洢�û���Ϣ
            session->setValue("userId", std::to_string(userId));
            session->setValue("username", username);
            session->setValue("isLoggedIn", "true");
            if (server_->onlineUsers_.find(userId) == server_->onlineUsers_.end() || server_->onlineUsers_[userId] == false)
            {
                {
                    std::lock_guard<std::mutex> lock(server_->mutexForOnlineUsers_);
                    server_->onlineUsers_[userId] = true;
                }

                // �û����ڵ�¼�ɹ�
                // ��װjson ���ݡ�
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
                // FIXME: ��ǰ���û����������ط���¼�У���ԭ�е�¼�û�ǿ�����߸���
                // �������ظ���¼��
                json failureResp;
                failureResp["success"] = false;
                failureResp["error"] = "�˺����������ط���¼";
                std::string failureBody = failureResp.dump(4);

                resp->setStatusLine(req.getVersion(), http::HttpResponse::k403Forbidden, "Forbidden");
                resp->setCloseConnection(true);
                resp->setContentType("application/json");
                resp->setContentLength(failureBody.size());
                resp->setBody(failureBody);
                return;
            }
        }
        else // �˺�������������µ�¼
        {
            // ��װjson����
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
        return;
    }

}

int ChatLoginHandler::queryUserId(const std::string& username, const std::string& password)
{
    // ǰ���û������˺����룬�������ݿ��Ƿ��и��˺�����
    // ʹ��Ԥ�������, ��ֹsqlע��
    std::string sql = "SELECT id FROM users WHERE username = ? AND password = ?";
    // std::vector<std::string> params = {username, password};
    auto res = mysqlUtil_.executeQuery(sql, username, password);
    if (res->next())
    {
        int id = res->getInt("id");
        return id;
    }
    // �����ѯ���Ϊ�գ��򷵻�-1
    return -1;
}

