#include "UserManager.h"

void UserManager::setCurrentUser(const UserInfo& userInfo) {
    _currentUser = userInfo;
    _isLoggedIn = true;
}

const UserInfo& UserManager::getCurrentUser() const {
    return _currentUser;
}

bool UserManager::isLoggedIn() const {
    return _isLoggedIn;
}

void UserManager::clearCurrentUser() {
    _currentUser = UserInfo();
    _isLoggedIn = false;
} 