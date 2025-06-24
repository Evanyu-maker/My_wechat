#pragma once
#include "UserDAO.h"
#include "../Singleton.h"
#include <memory>
#include <vector>

// 操作结果
enum class ResultCode {
    SUCCESS = 0,
    FAILED = 1,
    USER_ALREADY_EXISTS = 2,
    USER_NOT_FOUND = 3,
    INVALID_PASSWORD = 4,
    ALREADY_FRIENDS = 5,
    NOT_FRIENDS = 6,
    OPERATION_FAILED = 7,
    DATABASE_ERROR = 8
};

// 管理层操作结果
template <typename T>
class ManagerResult {
public:
    ManagerResult() : m_code(ResultCode::FAILED), m_data(nullptr) {}
    
    ManagerResult(ResultCode code) : m_code(code), m_data(nullptr) {}
    
    ManagerResult(ResultCode code, const std::string& message) 
        : m_code(code), m_message(message), m_data(nullptr) {}
    
    ManagerResult(ResultCode code, const std::string& message, std::shared_ptr<T> data) 
        : m_code(code), m_message(message), m_data(data) {}
    
    bool isSuccess() const { return m_code == ResultCode::SUCCESS; }
    
    ResultCode getCode() const { return m_code; }
    
    const std::string& getMessage() const { return m_message; }
    
    std::shared_ptr<T> getData() const { return m_data; }
    
private:
    ResultCode m_code;
    std::string m_message;
    std::shared_ptr<T> m_data;
};

// 无数据返回的结果特化
template <>
class ManagerResult<void> {
public:
    ManagerResult() : m_code(ResultCode::FAILED) {}
    
    ManagerResult(ResultCode code) : m_code(code) {}
    
    ManagerResult(ResultCode code, const std::string& message) 
        : m_code(code), m_message(message) {}
    
    bool isSuccess() const { return m_code == ResultCode::SUCCESS; }
    
    ResultCode getCode() const { return m_code; }
    
    const std::string& getMessage() const { return m_message; }
    
private:
    ResultCode m_code;
    std::string m_message;
};

class UserManager : public Singleton<UserManager> {
    friend class Singleton<UserManager>;
public:
    // 初始化
    bool init();
    
    // 用户注册
    ManagerResult<UserEntity> registerUser(const std::string& username, const std::string& password,
                     const std::string& nickname, const std::string& avatar);
    
    // 用户登录
    ManagerResult<UserEntity> login(const std::string& username, const std::string& password);
    
    // 用户登出
    ManagerResult<void> logout(int64_t userId);
    
    // 获取用户信息
    ManagerResult<UserEntity> getUserInfo(int64_t userId);
    
    // 获取用户信息通过用户名
    ManagerResult<UserEntity> getUserInfoByUsername(const std::string& username);
    
    // 获取好友列表
    ManagerResult<std::vector<UserEntity>> getFriendList(int64_t userId);
    
    // 更新用户状态
    ManagerResult<void> updateUserStatus(int64_t userId, const std::string& status);
    
    // 添加好友
    ManagerResult<void> addFriend(int64_t userId, int64_t friendId);
    
    // 删除好友
    ManagerResult<void> removeFriend(int64_t userId, int64_t friendId);
    
    // 批量获取用户状态
    ManagerResult<std::vector<UserEntity>> batchGetUserInfo(const std::vector<int64_t>& userIds);

private:
    UserManager() = default;
    UserDAO m_userDao;
}; 