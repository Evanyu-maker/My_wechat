#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QListWidget>
#include <QSplitter>
#include "global.h"
#include "ui_My_wechat.h"
#include "qtmaterialdrawer.h"
#include "qtmaterialappbar.h"
#include "qtmaterialsnackbar.h"
#include "qtmaterialtextfield.h"
#include "qtmaterialraisedbutton.h"
#include "qtmaterialiconbutton.h"
#include "qtmaterialfab.h"
#include "qtmaterialavatar.h"
#include "ChatBubble.h"
#include "ContactItem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class My_wechatClass; }
QT_END_NAMESPACE

class My_wechat : public QMainWindow
{
    Q_OBJECT
public:
    My_wechat(QWidget *parent = nullptr);
    ~My_wechat();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void sendMessage(); // 发送消息
    void contactSelected(int index); // 选择联系人
    void openEmoji(); // 打开表情面板
    void attachFile(); // 添加附件
    void showDrawer(); // 显示侧边抽屉

private:
    void setupChatUi(); // 设置聊天界面
    void addContactItem(const QString &name, const QString &lastMsg, const QString &avatarPath); // 添加联系人
    void addMessageBubble(const QString &message, ChatBubble::BubbleType type); // 添加消息气泡
    
    Ui::My_wechatClass ui;
    
    // 聊天界面组件
    QtMaterialDrawer *_drawer;
    QtMaterialAppBar *_appBar;
    QtMaterialSnackbar *_snackbar;
    
    QWidget *_centralWidget;
    QSplitter *_splitter;
    QListWidget *_contactList;
    QWidget *_chatWidget;
    QWidget *_chatHistoryWidget;
    QScrollArea *_chatScrollArea;
    QVBoxLayout *_chatHistoryLayout;
    QtMaterialTextField *_messageInput;
    QtMaterialRaisedButton *_sendButton;
    QtMaterialIconButton *_emojiButton;
    QtMaterialIconButton *_attachButton;
    QtMaterialFloatingActionButton *_newChatButton;
    
    QString _currentContact;
    bool _isMousePressed;
    QPoint _mousePos;
};
