#pragma once

// Qt基础组件
#include <QWidget>
#include <QMainWindow>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSplitter>
#include <QScrollArea>
#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QStyle>
#include <QDateTime>
#include <QRandomGenerator>
#include <QMessageBox>

// 标准库
#include <iostream>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>
#include <string>

// Qt-Material组件
// 确保以下路径与您的项目设置相符
#include "qtmaterialavatar.h"
#include "qtmaterialflatbutton.h"
#include "qtmaterialraisedbutton.h"
#include "qtmaterialscrollbar.h"
#include "qtmaterialtextfield.h"
#include "qtmaterialdrawer.h"
#include "qtmaterialappbar.h"
#include "qtmaterialiconbutton.h"
#include "qtmaterialsnackbar.h"
#include "qtmaterialfab.h"
#include "qtmaterialautocomplete.h"
#include <QMetaType>
/**
 * 用来书写全局的函数.
 */

/**
 * repolish用来刷新qss.
 */
extern std::function<void(QWidget)> repolish;

// 应用程序常量
const QString APP_NAME = "My_WeChat";
const QString APP_VERSION = "1.0.0";
const QString SERVER_URL = "http://localhost:8080";  // 服务器地址

// 错误码定义
enum class ErrorCodes {
    Success = 0,
    NetworkError = 1000,
    JsonError = 1001,
    ServerError = 1002,
    InvalidParams = 1003,
    UserNotExist = 2001,
    PasswordError = 2002,
    UserAlreadyExist = 2003,
    TokenInvalid = 2004
};

// 请求ID定义
enum class ReqId {
    None = 0,
    Register = 1,           // 注册请求
    Login = 2,             // 登录请求
    GetVerifyCode = 3,     // 获取验证码
    UpdateUserInfo = 4,    // 更新用户信息
    GetUserInfo = 5,       // 获取用户信息
    SendMessage = 6,       // 发送消息
    GetMessageHistory = 7  // 获取消息历史
};

// 模块定义
enum class Modules {
    None = 0,
    User = 1,    // 用户模块
    Chat = 2,    // 聊天模块
    Friend = 3,  // 好友模块
    Group = 4    // 群组模块
};

// 用户信息结构
struct UserInfo {
    QString userId;
    QString username;
    QString nickname;
    QString avatar;
    bool isOnline;
    
    UserInfo() : isOnline(false) {}
};

// 消息类型
enum class MessageType {
    Text = 0,
    Image = 1,
    File = 2,
    Voice = 3,
    Video = 4
};

// 全局函数命名空间
namespace GlobalFunctions {
    inline void logMessage(const QString& message) {
        // TODO: 实现日志记录功能
    }
}



