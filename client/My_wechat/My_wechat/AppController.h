#pragma once

#include <QObject>
#include "LoginDialog.h"
#include "RegisterDialog.h"
#include "My_wechat.h"

class AppController : public QObject
{
    Q_OBJECT

public:
    AppController(QObject *parent = nullptr);
    ~AppController();
    
    // 启动应用程序
    void start();

private slots:
    // 处理登录成功的槽函数
    void onLoginSuccess();
    
    // 处理切换到注册界面的槽函数
    void onSwitchToRegister();
    
    // 处理从注册返回登录界面的槽函数
    void onReturnToLogin();

private:
    LoginDialog* _loginDialog;
    RegisterDialog* _registerDialog;
    My_wechat* _mainWindow;
}; 