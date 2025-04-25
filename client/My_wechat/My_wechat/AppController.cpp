#include "AppController.h"
#include <QApplication>
#include <QScreen>

AppController::AppController(QObject *parent)
    : QObject(parent), _loginDialog(nullptr), _registerDialog(nullptr), _mainWindow(nullptr)
{
    // 创建登录对话框
    _loginDialog = new LoginDialog();
    
    // 创建注册对话框
    _registerDialog = new RegisterDialog();
    
    // 设置窗口标志，无边框
    _loginDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    _registerDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    
    // 连接信号和槽
    connect(_loginDialog, &LoginDialog::loginSuccess, this, &AppController::onLoginSuccess);
    connect(_loginDialog, &LoginDialog::switchRegiser, this, &AppController::onSwitchToRegister);
    connect(_registerDialog, &RegisterDialog::returnLogin, this, &AppController::onReturnToLogin);
}

AppController::~AppController()
{
    // 清理资源
    if (_loginDialog)
    {
        delete _loginDialog;
        _loginDialog = nullptr;
    }
    
    if (_registerDialog)
    {
        delete _registerDialog;
        _registerDialog = nullptr;
    }
    
    if (_mainWindow)
    {
        delete _mainWindow;
        _mainWindow = nullptr;
    }
}

void AppController::start()
{
    // 显示登录对话框
    _loginDialog->show();
    
    // 将窗口居中显示
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - _loginDialog->width()) / 2;
    int y = (screenGeometry.height() - _loginDialog->height()) / 2;
    _loginDialog->move(x, y);
}

void AppController::onLoginSuccess()
{
    // 隐藏登录对话框
    _loginDialog->hide();
    
    // 如果注册对话框存在，也隐藏它
    if (_registerDialog)
    {
        _registerDialog->hide();
    }
    
    // 创建并显示主窗口
    if (!_mainWindow)
    {
        _mainWindow = new My_wechat();
    }
    
    _mainWindow->show();
}

void AppController::onSwitchToRegister()
{
    // 隐藏登录对话框
    _loginDialog->hide();
    
    // 显示注册对话框
    _registerDialog->show();
    
    // 将窗口居中显示
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - _registerDialog->width()) / 2;
    int y = (screenGeometry.height() - _registerDialog->height()) / 2;
    _registerDialog->move(x, y);
}

void AppController::onReturnToLogin()
{
    // 隐藏注册对话框
    _registerDialog->hide();
    
    // 显示登录对话框
    _loginDialog->show();
} 