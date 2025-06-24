#pragma once

#include "global.h"
#include "Singleton.h"

/**
 * 用户管理器类
 * 使用单例模式存储和管理当前登录用户的信息
 */
class UserManager : public Singleton<UserManager>
{
public:
    // 设置当前用户信息
    void setCurrentUser(const UserInfo& userInfo);
    
    // 获取当前用户信息
    const UserInfo& getCurrentUser() const;
    
    // 检查用户是否已登录
    bool isLoggedIn() const;
    
    // 清除当前用户信息（登出时调用）
    void clearCurrentUser();

private:
    // 单例模式，为了能够在Singleton中使用，所以需要加上友元
    friend class Singleton<UserManager>;
    UserManager() : _isLoggedIn(false) ,{}
    
    UserInfo _currentUser;  // 当前登录用户的信息
    bool _isLoggedIn;       // 用户是否已登录的标志
    
}; 


// 在 UserManager.h 文件末尾添加（如果尚未添加）
#define g_userMgr UserManager::instance()