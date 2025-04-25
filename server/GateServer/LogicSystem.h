#pragma once
#include"Singleton.h"
#include "const.h"
class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
class LogicSystem : public Singleton<LogicSystem>	
{
    friend class Singleton<LogicSystem>;
public:
    ~LogicSystem() {};
    bool HandleGet(std::string url, std::shared_ptr<HttpConnection>);
    void RegGet(std::string url, HttpHandler handler);
	void RegPost(std::string url, HttpHandler handler);
    bool HandlePost(std::string url, std::shared_ptr<HttpConnection>);

private:
    LogicSystem();
    std::map<std::string, HttpHandler> _post_handlers;
    std::map<std::string, HttpHandler> _get_handlers;
}; 