#pragma once
#include "../../../../HttpServer/include/router/RouterHandler.h"
#include "../ChatServer.h"
#include "../../../HttpServer/include/utils/JsonUtil.h"

//ÒÑ¿´Íê
class ChatLogoutHandler : public http::router::RouterHandler
{
public:
    explicit ChatLogoutHandler(GomokuServer* server) : server_(server) {}
    void handle(const http::HttpRequest& req, http::HttpResponse* resp) override;
private:
    ChatServer* server_;
};