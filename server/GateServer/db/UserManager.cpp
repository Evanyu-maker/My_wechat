#include "UserManager.h"
#include "DBConnectionPool.h"
#include <iostream>

bool UserManager::init() {
    // Initialize any necessary components
    return true;
}

ManagerResult<UserEntity> UserManager::registerUser(const std::string& username, 
                                                 const std::string& password,
                                                 const std::string& nickname, 
                                                 const std::string& avatar) {
    // Check if username already exists
    auto findResult = m_userDao.findByUsername(username);
    if (findResult.isSuccess() && findResult.getData()) {
        return ManagerResult<UserEntity>(ResultCode::USER_ALREADY_EXISTS, "Username already exists");
    }
    
    // Create user entity
    UserEntity user;
    user.username = username;
    user.password = password;  // Note: In a real application, password should be encrypted
    user.nickname = nickname.empty() ? username : nickname;
    user.avatar = avatar.empty() ? "default.png" : avatar;
    user.status = "offline";  // Initial status is offline
    
    // Add user
    auto result = m_userDao.addUser(user);
    if (!result.isSuccess()) {
        return ManagerResult<UserEntity>(ResultCode::FAILED, result.getMessage());
    }
    
    // Get newly created user info
    auto newUserResult = m_userDao.findByUsername(username);
    if (!newUserResult.isSuccess()) {
        return ManagerResult<UserEntity>(ResultCode::SUCCESS, "User created successfully, but unable to get user details");
    }
    
    return ManagerResult<UserEntity>(ResultCode::SUCCESS, "User registered successfully", newUserResult.getData());
}

ManagerResult<UserEntity> UserManager::login(const std::string& username, 
                                          const std::string& password) {
    // Verify password
    auto verifyResult = m_userDao.verifyPassword(username, password);
    if (!verifyResult.isSuccess()) {
        return ManagerResult<UserEntity>(ResultCode::DATABASE_ERROR, verifyResult.getMessage());
    }
    
    if (!(*verifyResult.getData())) {
        return ManagerResult<UserEntity>(ResultCode::INVALID_PASSWORD, "Invalid username or password");
    }
    
    // Get user information
    auto userResult = m_userDao.findByUsername(username);
    if (!userResult.isSuccess() || !userResult.getData()) {
        return ManagerResult<UserEntity>(ResultCode::USER_NOT_FOUND, "User not found");
    }
    
    auto user = userResult.getData();
    
    // Update user status and login time
    m_userDao.updateUserStatus(user->userId, "online");  // 使用"online"表示在线状态
    m_userDao.updateLastLoginTime(user->userId);
    
    // Update user status in memory
    user->status = "online";
    
    return ManagerResult<UserEntity>(ResultCode::SUCCESS, "Login successful", user);
}

ManagerResult<void> UserManager::logout(int64_t userId) {
    auto result = m_userDao.updateUserStatus(userId, "offline");  // 0 means offline
    if (!result.isSuccess()) {
        return ManagerResult<void>(ResultCode::FAILED, result.getMessage());
    }
    
    return ManagerResult<void>(ResultCode::SUCCESS, "Logout successful");
}

ManagerResult<UserEntity> UserManager::getUserInfo(int64_t userId) {
    auto result = m_userDao.findById(userId);
    if (!result.isSuccess()) {
        return ManagerResult<UserEntity>(ResultCode::DATABASE_ERROR, result.getMessage());
    }
    
    if (!result.getData()) {
        return ManagerResult<UserEntity>(ResultCode::USER_NOT_FOUND, "User not found");
    }
    
    return ManagerResult<UserEntity>(ResultCode::SUCCESS, "User information retrieved successfully", result.getData());
}

ManagerResult<UserEntity> UserManager::getUserInfoByUsername(const std::string& username) {
    auto result = m_userDao.findByUsername(username);
    if (!result.isSuccess()) {
        return ManagerResult<UserEntity>(ResultCode::DATABASE_ERROR, result.getMessage());
    }
    
    if (!result.getData()) {
        return ManagerResult<UserEntity>(ResultCode::USER_NOT_FOUND, "User not found");
    }
    
    return ManagerResult<UserEntity>(ResultCode::SUCCESS, "User information retrieved successfully", result.getData());
}

ManagerResult<std::vector<UserEntity>> UserManager::getFriendList(int64_t userId) {
    auto result = m_userDao.getFriendList(userId);
    if (!result.isSuccess()) {
        return ManagerResult<std::vector<UserEntity>>(ResultCode::DATABASE_ERROR, result.getMessage());
    }
    
    return ManagerResult<std::vector<UserEntity>>(ResultCode::SUCCESS, "Friend list retrieved successfully", result.getData());
}

ManagerResult<void> UserManager::updateUserStatus(int64_t userId, const std::string& status) {
    auto result = m_userDao.updateUserStatus(userId, status);
    if (!result.isSuccess()) {
        return ManagerResult<void>(ResultCode::FAILED, result.getMessage());
    }
    
    return ManagerResult<void>(ResultCode::SUCCESS, "User status updated successfully");
}

ManagerResult<void> UserManager::addFriend(int64_t userId, int64_t friendId) {
    // Check if user exists
    auto userResult = m_userDao.findById(userId);
    if (!userResult.isSuccess() || !userResult.getData()) {
        return ManagerResult<void>(ResultCode::USER_NOT_FOUND, "User not found");
    }
    
    // Check if friend exists
    auto friendResult = m_userDao.findById(friendId);
    if (!friendResult.isSuccess() || !friendResult.getData()) {
        return ManagerResult<void>(ResultCode::USER_NOT_FOUND, "Friend not found");
    }
    
    // Add friend relationship
    auto result = m_userDao.addFriend(userId, friendId);
    if (!result.isSuccess()) {
        if (result.getMessage() == "Already friends") {
            return ManagerResult<void>(ResultCode::ALREADY_FRIENDS, result.getMessage());
        }
        return ManagerResult<void>(ResultCode::FAILED, result.getMessage());
    }
    
    return ManagerResult<void>(ResultCode::SUCCESS, "Friend added successfully");
}

ManagerResult<void> UserManager::removeFriend(int64_t userId, int64_t friendId) {
    // Remove friend relationship
    auto result = m_userDao.removeFriend(userId, friendId);
    if (!result.isSuccess()) {
        return ManagerResult<void>(ResultCode::FAILED, result.getMessage());
    }
    
    return ManagerResult<void>(ResultCode::SUCCESS, "Friend removed successfully");
}

ManagerResult<std::vector<UserEntity>> UserManager::batchGetUserInfo(const std::vector<int64_t>& userIds) {
    auto result = m_userDao.batchGetUserInfo(userIds);
    if (!result.isSuccess()) {
        return ManagerResult<std::vector<UserEntity>>(ResultCode::DATABASE_ERROR, result.getMessage());
    }
    
    return ManagerResult<std::vector<UserEntity>>(ResultCode::SUCCESS, "User information retrieved in batch successfully", result.getData());
} 