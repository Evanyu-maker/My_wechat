#pragma once

// Qt�������
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

// ��׼��
#include <iostream>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>
#include <string>

// Qt-Material���
// ȷ������·����������Ŀ�������
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
 * ������дȫ�ֵĺ���.
 */

/**
 * repolish����ˢ��qss.
 */
extern std::function<void(QWidget)> repolish;

// Ӧ�ó�����
const QString APP_NAME = "My_WeChat";
const QString APP_VERSION = "1.0.0";
const QString SERVER_URL = "http://localhost:8080";  // ��������ַ

// �����붨��
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

// ����ID����
enum class ReqId {
    None = 0,
    Register = 1,           // ע������
    Login = 2,             // ��¼����
    GetVerifyCode = 3,     // ��ȡ��֤��
    UpdateUserInfo = 4,    // �����û���Ϣ
    GetUserInfo = 5,       // ��ȡ�û���Ϣ
    SendMessage = 6,       // ������Ϣ
    GetMessageHistory = 7  // ��ȡ��Ϣ��ʷ
};

// ģ�鶨��
enum class Modules {
    None = 0,
    User = 1,    // �û�ģ��
    Chat = 2,    // ����ģ��
    Friend = 3,  // ����ģ��
    Group = 4    // Ⱥ��ģ��
};

// �û���Ϣ�ṹ
struct UserInfo {
    QString userId;
    QString username;
    QString nickname;
    QString avatar;
    bool isOnline;
    
    UserInfo() : isOnline(false) {}
};

// ��Ϣ����
enum class MessageType {
    Text = 0,
    Image = 1,
    File = 2,
    Voice = 3,
    Video = 4
};

// ȫ�ֺ��������ռ�
namespace GlobalFunctions {
    inline void logMessage(const QString& message) {
        // TODO: ʵ����־��¼����
    }
}



