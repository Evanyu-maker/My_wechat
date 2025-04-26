#include "LogicSystem.h"
#include"Httpconnection.h"
#include"VarifyGrpcClient.h"
#include"RedisMgr.h"
#include"db/DBManager.h"
#include "db/UserDAO.h"

LogicSystem::LogicSystem() {
    RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        beast::ostream(connection->_response.body()) << "receive get_test req " << std::endl;
        int i = 0;
        for (auto& elem : connection->_get_params) {
            i++;
            beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
            beast::ostream(connection->_response.body()) << ", " << " value is " << elem.second << std::endl;
        }
    });

    RegPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        connection->_response.set(http::field::content_type, "text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
            std::cout << "Failed to parse JSON data!" << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }

        auto email = src_root["email"].asString();
        GetVarifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVarifyCode(email);
        std::cout << "email is " << email << std::endl;
        root["error"] = rsp.error();
        root["email"] = src_root["email"];
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return true;
    });

    RegPost("/register", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
        std::cout << "receive register request body: " << body_str << std::endl;
        connection->_response.set(http::field::content_type, "text/json");
        
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
            std::cout << "Failed to parse JSON data!" << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }

        // 获取参数
        auto email = src_root["email"].asString();
        auto password = src_root["password"].asString();
        auto verify_code = src_root["verify_code"].asString();

        // 验证参数
        if (email.empty() || password.empty() || verify_code.empty()) {
            root["error"] = ErrorCodes::USER_REGISTER_FAILED;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }

        // 验证验证码
        auto redisMgr = RedisMgr::GetInstance();
        std::string stored_code;
        if (!redisMgr->Get("code_" + email, stored_code) || stored_code != verify_code) {
            root["error"] = ErrorCodes::USER_REGISTER_FAILED;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }

        // 创建用户实体
        UserEntity user;
        user.username = email;
        user.password = password;  // 注意：实际应用中应该对密码进行哈希处理
        user.nickname = email;     // 默认使用email作为nickname
        user.status = 1;          // 1: 正常状态

        UserDAO userDao;
        auto result = userDao.addUser(user);

        if (result.isSuccess()) {
            redisMgr->Del("code_" + email);
            root["error"] = ErrorCodes::Success;
        } else {
            if (result.getMessage().find("Duplicate entry") != std::string::npos) {
                root["error"] = ErrorCodes::USER_ALREADY_EXISTS;
            } else {
                root["error"] = ErrorCodes::USER_REGISTER_FAILED;
            }
        }

        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return true;
    });
}


void LogicSystem::RegGet(std::string url, HttpHandler handler) {
    _get_handlers.insert(make_pair(url, handler));
}

void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
    _post_handlers.insert(make_pair(url, handler));
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con)
{
    if (_post_handlers.find(path) == _post_handlers.end())
    {
        return false;
    }
    _post_handlers[path](con);
    return true;
}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con) {
    if (_get_handlers.find(path) == _get_handlers.end()) {
        return false;
    }
	//这里直接调用注册的函数。
    _get_handlers[path](con);
    return true;
}