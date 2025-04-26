#include "UserManager.h"
#include "DBConnectionPool.h"
#include <iostream>

bool UserManager::init() {
    // 在这里可以进行一些初始化操作
    return true;
}

ManagerResult<UserEntity> UserManager::registerUser(const std::string& username, 
                                                 const std::string& password,
                                                 const std::string& nickname, 
                                                 const std::string& avatar) {
    // 检查用户名是否已存在
    auto findResult = m_userDao.findByUsername(username);
    if (findResult.isSuccess() && findResult.getData()) {
        return ManagerResult<UserEntity>(ResultCode::USER_ALREADY_EXISTS, "用户名已存在");
    }
    
    // 创建用户实体
    UserEntity user;
    user.username = username;
    user.password = password;  // 注意：实际应用中应该对密码进行加密
    user.nickname = nickname.empty() ? username : nickname;
    user.avatar = avatar.empty() ? "default.png" : avatar;
    user.status = 0;  // 初始状态为离线
    
    // 添加用户
    auto result = m_userDao.addUser(user);
    if (!result.isSuccess()) {
        return ManagerResult<UserEntity>(ResultCode::FAILED, result.getMessage());
    }
    
    // 获取新创建的用户信息
    auto newUserResult = m_userDao.findByUsername(username);
    if (!newUserResult.isSuccess()) {
        return ManagerResult<UserEntity>(ResultCode::SUCCESS, "用户创建成功，但无法获取用户详情");
    }
    
    return ManagerResult<UserEntity>(ResultCode::SUCCESS, "用户注册成功", newUserResult.getData());
}

ManagerResult<UserEntity> UserManager::login(const std::string& username, 
                                          const std::string& password) {
    // 验证密码
    auto verifyResult = m_userDao.verifyPassword(username, password);
    if (!verifyResult.isSuccess()) {
        return ManagerResult<UserEntity>(ResultCode::DATABASE_ERROR, verifyResult.getMessage());
    }
    
    if (!(*verifyResult.getData())) {
        return ManagerResult<UserEntity>(ResultCode::INVALID_PASSWORD, "用户名或密码错误");
    }
    
    // 获取用户信息
    auto userResult = m_userDao.findByUsername(username);
    if (!userResult.isSuccess() || !userResult.getData()) {
        return ManagerResult<UserEntity>(ResultCode::USER_NOT_FOUND, "用户不存在");
    }
    
    auto user = userResult.getData();
    
    // 更新用户状态和登录时间
    m_userDao.updateUserStatus(user->userId, 1);  // 1表示在线
    m_userDao.updateLastLoginTime(user->userId);
    
    // 更新内存中的用户状态
    user->status = 1;
    
    return ManagerResult<UserEntity>(ResultCode::SUCCESS, "登录成功", user);
}

ManagerResult<void> UserManager::logout(int64_t userId) {
    auto result = m_userDao.updateUserStatus(userId, 0);  // 0表示离线
    if (!result.isSuccess()) {
        return ManagerResult<void>(ResultCode::FAILED, result.getMessage());
    }
    
    return ManagerResult<void>(ResultCode::SUCCESS, "登出成功");
}

ManagerResult<UserEntity> UserManager::getUserInfo(int64_t userId) {
    auto result = m_userDao.findById(userId);
    if (!result.isSuccess()) {
        return ManagerResult<UserEntity>(ResultCode::DATABASE_ERROR, result.getMessage());
    }
    
    if (!result.getData()) {
        return ManagerResult<UserEntity>(ResultCode::USER_NOT_FOUND, "用户不存在");
    }
    
    return ManagerResult<UserEntity>(ResultCode::SUCCESS, "获取用户信息成功", result.getData());
}

ManagerResult<UserEntity> UserManager::getUserInfoByUsername(const std::string& username) {
    auto result = m_userDao.findByUsername(username);
    if (!result.isSuccess()) {
        return ManagerResult<UserEntity>(ResultCode::DATABASE_ERROR, result.getMessage());
    }
    
    if (!result.getData()) {
        return ManagerResult<UserEntity>(ResultCode::USER_NOT_FOUND, "用户不存在");
    }
    
    return ManagerResult<UserEntity>(ResultCode::SUCCESS, "获取用户信息成功", result.getData());
}

ManagerResult<std::vector<UserEntity>> UserManager::getFriendList(int64_t userId) {
    auto result = m_userDao.getFriendList(userId);
    if (!result.isSuccess()) {
        return ManagerResult<std::vector<UserEntity>>(ResultCode::DATABASE_ERROR, result.getMessage());
    }
    
    return ManagerResult<std::vector<UserEntity>>(ResultCode::SUCCESS, "获取好友列表成功", result.getData());
}

ManagerResult<void> UserManager::updateUserStatus(int64_t userId, int status) {
    auto result = m_userDao.updateUserStatus(userId, status);
    if (!result.isSuccess()) {
        return ManagerResult<void>(ResultCode::FAILED, result.getMessage());
    }
    
    return ManagerResult<void>(ResultCode::SUCCESS, "更新用户状态成功");
}

ManagerResult<void> UserManager::addFriend(int64_t userId, int64_t friendId) {
    // 检查用户是否存在
    auto userResult = m_userDao.findById(userId);
    if (!userResult.isSuccess() || !userResult.getData()) {
        return ManagerResult<void>(ResultCode::USER_NOT_FOUND, "用户不存在");
    }
    
    // 检查好友是否存在
    auto friendResult = m_userDao.findById(friendId);
    if (!friendResult.isSuccess() || !friendResult.getData()) {
        return ManagerResult<void>(ResultCode::USER_NOT_FOUND, "好友不存在");
    }
    
    // 添加好友关系
    auto result = m_userDao.addFriend(userId, friendId);
    if (!result.isSuccess()) {
        if (result.getMessage() == "已经是好友关系") {
            return ManagerResult<void>(ResultCode::ALREADY_FRIENDS, result.getMessage());
        }
        return ManagerResult<void>(ResultCode::FAILED, result.getMessage());
    }
    
    return ManagerResult<void>(ResultCode::SUCCESS, "添加好友成功");
}

ManagerResult<void> UserManager::removeFriend(int64_t userId, int64_t friendId) {
    // 删除好友关系
    auto result = m_userDao.removeFriend(userId, friendId);
    if (!result.isSuccess()) {
        return ManagerResult<void>(ResultCode::FAILED, result.getMessage());
    }
    
    return ManagerResult<void>(ResultCode::SUCCESS, "删除好友成功");
}

ManagerResult<std::vector<UserEntity>> UserManager::batchGetUserInfo(const std::vector<int64_t>& userIds) {
    auto result = m_userDao.batchGetUserInfo(userIds);
    if (!result.isSuccess()) {
        return ManagerResult<std::vector<UserEntity>>(ResultCode::DATABASE_ERROR, result.getMessage());
    }
    
    return ManagerResult<std::vector<UserEntity>>(ResultCode::SUCCESS, "批量获取用户信息成功", result.getData());
} 