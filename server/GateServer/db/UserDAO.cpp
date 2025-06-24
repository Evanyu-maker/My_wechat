#include "UserDAO.h"
#include <sstream>

DAOResult<void> UserDAO::addUser(const UserEntity& user) {
    DAO_TRY
        auto connWrapper = getConnection();
        auto conn = connWrapper->getConnection();
        
        // Call add user procedure
        auto stmt = prepareProcedureCall("proc_add_user", 6, conn);
        stmt->setString(1, user.username);
        stmt->setString(2, user.password);
        stmt->setString(3, user.nickname);
        stmt->setString(4, user.avatar);
        stmt->setString(5, user.email);
        stmt->setString(6, user.status);
        
        stmt->execute();
        
        return DAOResult<void>(true, "User added successfully");
    DAO_CATCH(void)
}

DAOResult<UserEntity> UserDAO::findByUsername(const std::string& username) {
    DAO_TRY
        auto connWrapper = getConnection();
        auto conn = connWrapper->getConnection();
        
        // Call find user by username procedure
        auto stmt = prepareProcedureCall("proc_find_user_by_username", 1, conn);
        stmt->setString(1, username);
        
        auto rs = std::shared_ptr<sql::ResultSet>(stmt->executeQuery());
        if (rs->next()) {
            auto user = std::make_shared<UserEntity>(buildUserFromResultSet(rs.get()));
            return DAOResult<UserEntity>(true, "User found", user);
        }
        return DAOResult<UserEntity>(false, "User not found");
    DAO_CATCH(UserEntity)
}

DAOResult<UserEntity> UserDAO::findById(int64_t userId) {
    DAO_TRY
        auto connWrapper = getConnection();
        auto conn = connWrapper->getConnection();
        
        // Call find user by ID procedure
        auto stmt = prepareProcedureCall("proc_find_user_by_id", 1, conn);
        stmt->setInt64(1, userId);
        
        auto rs = std::shared_ptr<sql::ResultSet>(stmt->executeQuery());
        if (rs->next()) {
            auto user = std::make_shared<UserEntity>(buildUserFromResultSet(rs.get()));
            return DAOResult<UserEntity>(true, "User found", user);
        }
        return DAOResult<UserEntity>(false, "User not found");
    DAO_CATCH(UserEntity)
}

DAOResult<void> UserDAO::updateUserStatus(int64_t userId, const std::string& status) {
    DAO_TRY
        auto connWrapper = getConnection();
        auto conn = connWrapper->getConnection();
        
        // Call update user status procedure
        auto stmt = prepareProcedureCall("proc_update_user_status", 2, conn);
        stmt->setInt64(1, userId);
        stmt->setString(2, status);
        
        stmt->execute();
        
        return DAOResult<void>(true, "Status updated");
    DAO_CATCH(void)
}

DAOResult<void> UserDAO::updateLastLoginTime(int64_t userId) {
    DAO_TRY
        auto connWrapper = getConnection();
        auto conn = connWrapper->getConnection();
        
        // Call update login time procedure
        auto stmt = prepareProcedureCall("proc_update_last_login_time", 1, conn);
        stmt->setInt64(1, userId);
        
        stmt->execute();
        
        return DAOResult<void>(true, "Login time updated");
    DAO_CATCH(void)
}

DAOResult<std::vector<UserEntity>> UserDAO::getFriendList(int64_t userId) {
    DAO_TRY
        auto connWrapper = getConnection();
        auto conn = connWrapper->getConnection();
        
        // Call get friend list procedure
        auto stmt = prepareProcedureCall("proc_get_friend_list", 1, conn);
        stmt->setInt64(1, userId);
        
        auto rs = std::shared_ptr<sql::ResultSet>(stmt->executeQuery());
        std::vector<UserEntity> friends;
        
        while (rs->next()) {
            friends.push_back(buildUserFromResultSet(rs.get()));
        }
        
        auto result = std::make_shared<std::vector<UserEntity>>(std::move(friends));
        return DAOResult<std::vector<UserEntity>>(true, "Friend list retrieved", result);
    DAO_CATCH(std::vector<UserEntity>)
}

DAOResult<bool> UserDAO::verifyPassword(const std::string& username, const std::string& password) {
    DAO_TRY
        auto connWrapper = getConnection();
        auto conn = connWrapper->getConnection();
        
        // Call verify password procedure
        auto stmt = prepareProcedureCall("proc_verify_password", 2, conn);
        stmt->setString(1, username);
        stmt->setString(2, password);
        
        auto rs = std::shared_ptr<sql::ResultSet>(stmt->executeQuery());
        bool verified = false;
        
        if (rs->next()) {
            verified = rs->getInt("result") > 0;
        }
        
        auto result = std::make_shared<bool>(verified);
        return DAOResult<bool>(true, verified ? "Password verified" : "Password incorrect", result);
    DAO_CATCH(bool)
}

DAOResult<void> UserDAO::addFriend(int64_t userId, int64_t friendId) {
    DAO_TRY
        auto connWrapper = getConnection();
        auto conn = connWrapper->getConnection();
        
        // Execute transaction to ensure atomicity
        return executeTransaction([this, userId, friendId](sql::Connection* conn) -> DAOResult<void> {
            // Call add friend procedure
            auto stmt = prepareProcedureCall("proc_add_friend", 2, conn);
            stmt->setInt64(1, userId);
            stmt->setInt64(2, friendId);
            
            stmt->execute();
            
            return DAOResult<void>(true, "Friend added");
        });
    DAO_CATCH(void)
}

DAOResult<void> UserDAO::removeFriend(int64_t userId, int64_t friendId) {
    DAO_TRY
        auto connWrapper = getConnection();
        auto conn = connWrapper->getConnection();
        
        // Call remove friend procedure
        auto stmt = prepareProcedureCall("proc_remove_friend", 2, conn);
        stmt->setInt64(1, userId);
        stmt->setInt64(2, friendId);
        
        stmt->execute();
        
        return DAOResult<void>(true, "Friend removed");
    DAO_CATCH(void)
}

DAOResult<std::vector<UserEntity>> UserDAO::batchGetUserInfo(const std::vector<int64_t>& userIds) {
    if (userIds.empty()) {
        return DAOResult<std::vector<UserEntity>>(true, "User ID list is empty", 
                                               std::make_shared<std::vector<UserEntity>>());
    }
    
    DAO_TRY
        auto connWrapper = getConnection();
        auto conn = connWrapper->getConnection();
        
        // Build user ID list string
        std::string idListStr = buildUserIdList(userIds);
        
        // Call batch get user info procedure
        auto stmt = prepareProcedureCall("proc_batch_get_user_info", 1, conn);
        stmt->setString(1, idListStr);
        
        auto rs = std::shared_ptr<sql::ResultSet>(stmt->executeQuery());
        std::vector<UserEntity> users;
        
        while (rs->next()) {
            users.push_back(buildUserFromResultSet(rs.get()));
        }
        
        auto result = std::make_shared<std::vector<UserEntity>>(std::move(users));
        return DAOResult<std::vector<UserEntity>>(true, "Users retrieved", result);
    DAO_CATCH(std::vector<UserEntity>)
}

std::string UserDAO::buildUserIdList(const std::vector<int64_t>& userIds) {
    std::stringstream ss;
    for (size_t i = 0; i < userIds.size(); i++) {
        if (i > 0) {
            ss << ",";
        }
        ss << userIds[i];
    }
    return ss.str();
}

UserEntity UserDAO::buildUserFromResultSet(sql::ResultSet* rs) {
    UserEntity user;
    user.userId = rs->getInt64("user_id");
    user.username = rs->getString("username");
    user.password = rs->getString("password");
    user.nickname = rs->getString("nickname");
    user.avatar = rs->getString("avatar");
    user.email = rs->getString("email");
    user.status = rs->getString("status");
    user.createTime = rs->getString("create_time");
    user.lastLoginTime = rs->getString("last_login_time");
    return user;
} 