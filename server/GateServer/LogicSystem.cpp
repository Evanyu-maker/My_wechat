#include "LogicSystem.h"
#include"Httpconnection.h"
#include"VarifyGrpcClient.h"
#include"RedisMgr.h"
#include"db/DBManager.h"
#include "db/UserDAO.h"
#include "db/UserManager.h"

LogicSystem::LogicSystem() {
    // ��ʼ�����ݿ����ӳ�
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

        // ��ȡ����
        auto username = src_root["username"].asString();
        auto password = src_root["password"].asString();

        // ��֤����
        if (username.empty() || password.empty()) {
            root["error"] = ErrorCodes::InvalidParams;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }

        // ��֤�û���������
        auto result = UserManager::GetInstance()->login(username, password);
        
        if (result.getCode() == ResultCode::SUCCESS) {
            // ��¼�ɹ�
            auto user = result.getData();
            root["error"] = ErrorCodes::Success;
            
            // ���û���Ϣ��װ��userInfo������
            Json::Value userInfo;
            userInfo["userId"] = Json::Int(user->userId);
            userInfo["username"] = user->username;
            userInfo["nickname"] = user->nickname;
            userInfo["avatar"] = user->avatar;
            userInfo["email"] = user->email;
            userInfo["status"] = user->status;
            
            root["userInfo"] = userInfo;
            
            // ��������������token���ͻ��ˣ��Ա����������֤
            // TODO: ���token���ɺ͹����߼�
        } else if (result.getCode() == ResultCode::INVALID_PASSWORD) {
            // �������
            root["error"] = ErrorCodes::USER_INVALID_PASSWORD;  // �������˴�����
        } else if (result.getCode() == ResultCode::USER_NOT_FOUND) {
            // �û�������
            root["error"] = ErrorCodes::USER_NOT_FOUND;  // �������˴�����
        } else {
            // ��������
            std::cout << "��¼ʧ�ܣ�������Ϣ: " << result.getMessage() << std::endl;
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

        // ��ȡ����
        auto username = src_root["username"].asString();
        auto email = src_root["email"].asString();
        auto password = src_root["password"].asString();
        auto verify_code = src_root["verify_code"].asString();

        // ��֤����
        if (username.empty() || email.empty() || password.empty() || verify_code.empty()) {
            root["error"] = ErrorCodes::InvalidParams;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }

        // ��֤��֤��
        auto redisMgr = RedisMgr::GetInstance();
        std::string stored_code;
        if (!redisMgr->Get("code:" + email, stored_code)) {
            root["error"] = ErrorCodes::TokenInvalid;  // ��֤�벻���ڻ��ѹ���
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }
        
        if (stored_code != verify_code) {
            root["error"] = ErrorCodes::TokenInvalid;  // ��֤�벻ƥ��
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }

        // �����û�ʵ��
        UserEntity user;
        user.username = username;  // ʹ�ÿͻ��˴������û���
        user.password = password;  // ע�⣺ʵ��Ӧ����Ӧ�ö�������й�ϣ����
        user.nickname = username;  // Ĭ��ʹ��username��Ϊnickname
        user.email = email;        // ����������Ϣ
        user.status = "offline";   // ʹ���ַ������͵�״̬���������ݿ�ENUM����
        user.avatar = "default.png";  // ����Ĭ��ͷ�񣬱����ֵ

        UserDAO userDao;
        auto result = userDao.addUser(user);

        if (result.isSuccess()) {
            redisMgr->Del("code:" + email);  // ע��ɹ���ɾ����֤��
            root["error"] = ErrorCodes::Success;
        } else {
            if (result.getMessage().find("Duplicate entry") != std::string::npos) {
                root["error"] = ErrorCodes::USER_ALREADY_EXISTS;
            } else {
                // �����ϸ�Ĵ�����־
                std::cout << "�û�ע��ʧ�ܣ�������Ϣ: " << result.getMessage() << std::endl;
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
	//����ֱ�ӵ���ע��ĺ�����
    _get_handlers[path](con);
    return true;
}