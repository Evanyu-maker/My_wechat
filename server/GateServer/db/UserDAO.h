#pragma once
#include "BaseDAO.h"
#include <vector>
#include <optional>

// 用户实体类
struct UserEntity {
    int64_t userId;
    std::string username;
    std::string password;
    std::string nickname;
    std::string avatar;
    int status;
    std::string createTime;
    std::string lastLoginTime;
};

class UserDAO : public BaseDAO {
public:
    // 添加用户 (proc_add_user)
    // 参数: username, password, nickname, avatar, status
    DAOResult<void> addUser(const UserEntity& user);
    
    // 通过用户名查找用户 (proc_find_user_by_username)
    // 参数: username
    DAOResult<UserEntity> findByUsername(const std::string& username);
    
    // 通过用户ID查找用户 (proc_find_user_by_id)
    // 参数: user_id
    DAOResult<UserEntity> findById(int64_t userId);
    
    // 更新用户状态 (proc_update_user_status)
    // 参数: user_id, status
    DAOResult<void> updateUserStatus(int64_t userId, int status);
    
    // 更新最后登录时间 (proc_update_last_login_time)
    // 参数: user_id
    DAOResult<void> updateLastLoginTime(int64_t userId);
    
    // 获取用户好友列表 (proc_get_friend_list)
    // 参数: user_id
    DAOResult<std::vector<UserEntity>> getFriendList(int64_t userId);
    
    // 验证用户密码 (proc_verify_password)
    // 参数: username, password
    DAOResult<bool> verifyPassword(const std::string& username, const std::string& password);
    
    // 添加好友关系 (proc_add_friend)
    // 参数: user_id, friend_id
    DAOResult<void> addFriend(int64_t userId, int64_t friendId);
    
    // 删除好友关系 (proc_remove_friend)
    // 参数: user_id, friend_id
    DAOResult<void> removeFriend(int64_t userId, int64_t friendId);
    
    // 批量获取用户信息 (proc_batch_get_user_info)
    // 参数: user_ids (逗号分隔的ID列表)
    DAOResult<std::vector<UserEntity>> batchGetUserInfo(const std::vector<int64_t>& userIds);
    
private:
    // 从结果集构造用户实体
    UserEntity buildUserFromResultSet(sql::ResultSet* rs);
    
    // 构建用户ID列表字符串
    std::string buildUserIdList(const std::vector<int64_t>& userIds);
}; 