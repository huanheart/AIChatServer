#pragma once
#include "../../../../HttpServer/include/router/RouterHandler.h"
#include "../../../HttpServer/include/utils/MysqlUtil.h"
#include "../ChatServer.h"

class ChatHandler : public http::router::RouterHandler
{
public:
    explicit ChatHandler(GomokuServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse* resp) override;
private:

private:
    GomokuServer* server_;
    http::MysqlUtil     mysqlUtil_;
};
