#include "LogicSystem.h"
#include"Httpconnection.h"
#include"VarifyGrpcClient.h"
#include"RedisMgr.h"
#include"db/DBManager.h"
#include "db/UserDAO.h"
#include "db/UserManager.h"

LogicSystem::LogicSystem() {
    // 初始化数据库连接池
    if (!gDBManager.initDBConnectionPool()) {
        std::cerr << "Failed to initialize database connection pool" << std::endl;
        throw std::runtime_error("Database connection pool initialization failed");
    }
    
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

    RegPost("/login", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
        std::cout << "receive login request body: " << body_str << std::endl;
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
        auto username = src_root["username"].asString();
        auto password = src_root["password"].asString();

        // 验证参数
        if (username.empty() || password.empty()) {
            root["error"] = ErrorCodes::InvalidParams;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }

        // 验证用户名和密码
        auto result = UserManager::GetInstance()->login(username, password);
        
        if (result.getCode() == ResultCode::SUCCESS) {
            // 登录成功
            auto user = result.getData();
            root["error"] = ErrorCodes::Success;
            
            // 将用户信息封装到userInfo对象中
            Json::Value userInfo;
            userInfo["userId"] = Json::Int(user->userId);
            userInfo["username"] = user->username;
            userInfo["nickname"] = user->nickname;
            userInfo["avatar"] = user->avatar;
            userInfo["email"] = user->email;
            userInfo["status"] = user->status;
            
            root["userInfo"] = userInfo;
            
            // 可以在这里生成token给客户端，以便后续请求验证
            // TODO: 添加token生成和管理逻辑
        } else if (result.getCode() == ResultCode::INVALID_PASSWORD) {
            // 密码错误
            root["error"] = ErrorCodes::USER_INVALID_PASSWORD;  // 服务器端错误码
        } else if (result.getCode() == ResultCode::USER_NOT_FOUND) {
            // 用户不存在
            root["error"] = ErrorCodes::USER_NOT_FOUND;  // 服务器端错误码
        } else {
            // 其他错误
            std::cout << "登录失败，错误信息: " << result.getMessage() << std::endl;
            root["error"] = ErrorCodes::USER_LOGIN_FAILED;
        }

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
        auto username = src_root["username"].asString();
        auto email = src_root["email"].asString();
        auto password = src_root["password"].asString();
        auto verify_code = src_root["verify_code"].asString();

        // 验证参数
        if (username.empty() || email.empty() || password.empty() || verify_code.empty()) {
            root["error"] = ErrorCodes::InvalidParams;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }

        // 验证验证码
        auto redisMgr = RedisMgr::GetInstance();
        std::string stored_code;
        if (!redisMgr->Get("code:" + email, stored_code)) {
            root["error"] = ErrorCodes::TokenInvalid;  // 验证码不存在或已过期
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }
        
        if (stored_code != verify_code) {
            root["error"] = ErrorCodes::TokenInvalid;  // 验证码不匹配
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }

        // 创建用户实体
        UserEntity user;
        user.username = username;  // 使用客户端传来的用户名
        user.password = password;  // 注意：实际应用中应该对密码进行哈希处理
        user.nickname = username;  // 默认使用username作为nickname
        user.email = email;        // 保存邮箱信息
        user.status = "offline";   // 使用字符串类型的状态，符合数据库ENUM定义
        user.avatar = "default.png";  // 设置默认头像，避免空值

        UserDAO userDao;
        auto result = userDao.addUser(user);

        if (result.isSuccess()) {
            redisMgr->Del("code:" + email);  // 注册成功后删除验证码
            root["error"] = ErrorCodes::Success;
        } else {
            if (result.getMessage().find("Duplicate entry") != std::string::npos) {
                root["error"] = ErrorCodes::USER_ALREADY_EXISTS;
            } else {
                // 添加详细的错误日志
                std::cout << "用户注册失败，错误信息: " << result.getMessage() << std::endl;
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