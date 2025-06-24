#include "My_wechat.h"
#include "ui_My_wechat.h"
#include "ChatBubble.h"
#include "ContactItem.h"
#include "UserManager.h"
#include <QFile>
#include <QPixmap>
#include <QTimer>
#include <QRandomGenerator>
#include <QScrollBar>
#include "qtmaterialscrollbar.h"
#include "qtmaterialflatbutton.h"
#include "qtmaterialfab.h"
#include "qtmaterialsnackbar.h"
#include "qtmaterialdrawer.h"
#include "qtmaterialappbar.h"
#include "qtmaterialiconbutton.h"
#include "qtmaterialtextfield.h"
#include "qtmaterialraisedbutton.h"
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>

My_wechat::My_wechat(QWidget *parent)
    : QMainWindow(parent),
    _windowMoving(false),
    _isDrawerOpen(false)
{
    ui.setupUi(this);
    
    // 设置窗口标志，允许最大化、最小化和关闭
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    // 加载样式表
    QFile styleFile(":/style/stylesheet.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = styleFile.readAll();
        setStyleSheet(styleSheet);
        styleFile.close();
    }
    
    // 设置聊天界面
    setupChatUi();
    
    // 加载当前用户信息
    loadUserInfo();
}

void My_wechat::setupChatUi()
{
    setWindowTitle("Telegram");
    resize(950, 1000);
    setMinimumSize(950, 1000);
    
    // 创建主窗口部件
    _centralWidget = new QWidget(this);
    setCentralWidget(_centralWidget);
    
    // 创建 Material Design 风格的顶部栏
    _appBar = new QtMaterialAppBar(this);
    _appBar->setBackgroundColor(QColor("#2AABEE"));
    _appBar->setFixedHeight(64);
    
    // 创建标题文本
    QLabel *titleLabel = new QLabel("Telegram", _appBar);
    titleLabel->setObjectName("titleLabel");  // 为了后面好找到这个标签
    titleLabel->setStyleSheet(
        "QLabel {"
        "    color: white;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "}"
    );
    titleLabel->setAlignment(Qt::AlignCenter);
    _appBar->appBarLayout()->addWidget(titleLabel);
    _appBar->appBarLayout()->addStretch(1);
    
    // 创建右上角按钮容器
    QWidget *rightButtons = new QWidget(_appBar);
    QHBoxLayout *rightButtonsLayout = new QHBoxLayout(rightButtons);
    rightButtonsLayout->setContentsMargins(4, 0, 4, 0);
    rightButtonsLayout->setSpacing(8);  // 增加按钮之间的间距
    
    // 统一按钮样式的函数
    auto setupWindowButton = [](QtMaterialIconButton* btn, const QString& iconPath) {
        btn->setFixedSize(32, 32);  // 设置固定大小
        btn->setIconSize(QSize(20, 20));  // 设置图标大小
        btn->setStyleSheet(
            "QtMaterialIconButton {"
            "    color: white;"
            "    margin: 0;"
            "    padding: 6px;"
            "    border: none;"
            "    border-radius: 0px;"
            "}"
            "QtMaterialIconButton:hover {"
            "    background-color: rgba(255,255,255,0.1);"
            "}"
        );
    };
    
    // 添加最小化按钮
    QtMaterialIconButton *minButton = new QtMaterialIconButton(QIcon(":/res/Min_.png"), rightButtons);
    setupWindowButton(minButton, ":/res/Min_.png");
    connect(minButton, &QtMaterialIconButton::clicked, this, &QWidget::showMinimized);
    
    // 添加最大化按钮
    QtMaterialIconButton *maxButton = new QtMaterialIconButton(QIcon(":/res/Max_.png"), rightButtons);
    setupWindowButton(maxButton, ":/res/Max_.png");
    connect(maxButton, &QtMaterialIconButton::clicked, this, [this, maxButton]() {
        if (isMaximized()) {
            showNormal();
            maxButton->setIcon(QIcon(":/res/Restore_.png"));
        } else {
            showMaximized();
            maxButton->setIcon(QIcon(":/res/Restore_.png"));
        }
    });
    
    // 添加关闭按钮
    QtMaterialIconButton *closeButton = new QtMaterialIconButton(QIcon(":/res/Close_.png"), rightButtons);
    setupWindowButton(closeButton, ":/res/Close_.png");
    closeButton->setStyleSheet(
        "QtMaterialIconButton {"
        "    color: white;"
        "    margin: 0;"
        "    padding: 6px;"
        "    border: none;"
        "    border-radius: 0px;"
        "}"
        "QtMaterialIconButton:hover {"
        "    background-color: #F44336;"
        "}"
    );
    connect(closeButton, &QtMaterialIconButton::clicked, this, &QWidget::close);
    
    // 将按钮添加到布局中
    rightButtonsLayout->addWidget(minButton);
    rightButtonsLayout->addWidget(maxButton);
    rightButtonsLayout->addWidget(closeButton);
    
    rightButtons->setLayout(rightButtonsLayout);
    _appBar->appBarLayout()->addWidget(rightButtons);
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(_centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(_appBar);
    
    // 内容容器
    QWidget *contentContainer = new QWidget(_centralWidget);
    QHBoxLayout *contentLayout = new QHBoxLayout(contentContainer);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    
    // =============== 最左侧窄条导航栏 ===============
    QWidget *navBarWidget = new QWidget(contentContainer);
    navBarWidget->setFixedWidth(60);
    navBarWidget->setStyleSheet(
        "background-color: #2C3E50;"
        "border-right: 1px solid #243342;" // 添加右侧边框，增强分隔感
    );
    
    QVBoxLayout *navBarLayout = new QVBoxLayout(navBarWidget);
    navBarLayout->setContentsMargins(0, 15, 0, 10);
    navBarLayout->setSpacing(20);
    navBarLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    
    // 创建侧边栏按钮的辅助函数
    auto createNavButton = [navBarWidget](const QString &iconPath, const QString &tooltip, int size = 36) { // 减小按钮尺寸
        QtMaterialIconButton *btn = new QtMaterialIconButton(QIcon(iconPath), navBarWidget);
        btn->setFixedSize(size, size);
        btn->setIconSize(QSize(size-12, size-12)); // 调整图标大小
        btn->setToolTip(tooltip);
        btn->setStyleSheet(
            "QtMaterialIconButton {"
            "    color: white;"
            "    background-color: transparent;"
            "    border-radius: 18px;" // 调整圆角以匹配尺寸
            "}"
            "QtMaterialIconButton:hover {"
            "    background-color: rgba(255,255,255,0.2);"
            "}"
            "QtMaterialIconButton:pressed {"
            "    background-color: rgba(255,255,255,0.3);"
            "}"
        );
        return btn;
    };
    
    // 微信Logo显示
    QLabel *appLogoLabel = new QLabel(navBarWidget);
    appLogoLabel->setPixmap(QPixmap(":/res/telegram.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    appLogoLabel->setAlignment(Qt::AlignCenter);
    appLogoLabel->setFixedHeight(30);
    appLogoLabel->setStyleSheet("margin-top: 8px; margin-bottom: 10px;");
    
    // 抽屉菜单按钮 - 位于Logo下方
    QtMaterialIconButton *drawerButton = createNavButton(":/res/menu.png", "菜单");
    connect(drawerButton, &QtMaterialIconButton::clicked, this, &My_wechat::showDrawer);
    
    // 添加Logo和菜单按钮到布局
    navBarLayout->addWidget(appLogoLabel);
    navBarLayout->addWidget(drawerButton);
    navBarLayout->addSpacing(10);
    
    // 使用聊天图标按钮代替头像，保持与其他图标一致性
    QtMaterialIconButton *chatButton = createNavButton(":/res/default-avatar.png", "聊天");
    chatButton->setObjectName("chatButton"); // 设置对象名称以便于查找
    chatButton->setStyleSheet(
        "QtMaterialIconButton#chatButton {"
        "    color: white;"
        "    background-color: #3498DB;" // 使用蓝色背景突出显示当前选中状态
        "    border-radius: 18px;"
        "}"
        "QtMaterialIconButton#chatButton:hover {"
        "    background-color: #2980B9;"
        "}"
        "QtMaterialIconButton#chatButton:pressed {"
        "    background-color: #1F6AA5;"
        "}"
    );
    
    // 连接聊天按钮点击信号
    connect(chatButton, &QtMaterialIconButton::clicked, this, [this]() {
        // 确保联系人列表是可见的
        if (_contactList && _contactList->count() > 0) {
            _contactList->setCurrentRow(0); // 默认选择第一个联系人
            contactSelected(0);
        }
        
        // 通知用户
        _snackbar->addMessage("切换到聊天列表");
    });
    
    // 未读消息标签的容器
    QWidget *chatBtnContainer = new QWidget(navBarWidget);
    chatBtnContainer->setFixedSize(36, 36);
    chatButton->setParent(chatBtnContainer);
    chatButton->move(0, 0);
    
    // 未读消息标签
    QLabel *unreadBadge = new QLabel("25", chatBtnContainer);
    unreadBadge->setFixedSize(16, 16);
    unreadBadge->setAlignment(Qt::AlignCenter);
    unreadBadge->setStyleSheet(
        "QLabel {"
        "    background-color: #E74C3C;"
        "    color: white;"
        "    border-radius: 8px;"
        "    font-size: 10px;"
        "    font-weight: bold;"
        "    padding: 0px;"
        "    border: 1px solid #2C3E50;" // 添加边框使其更加突出
        "}"
    );
    unreadBadge->move(20, 0); // 右上角位置
    
    // 其他导航按钮
    QtMaterialIconButton *contactsBtn = createNavButton(":/res/group.png", "联系人");
    contactsBtn->setObjectName("contactsBtn");
    contactsBtn->setStyleSheet(
        "QtMaterialIconButton#contactsBtn {"
        "    color: white;"
        "    background-color: transparent;"
        "    border-radius: 18px;"
        "}"
        "QtMaterialIconButton#contactsBtn:hover {"
        "    background-color: rgba(255,255,255,0.2);"
        "}"
        "QtMaterialIconButton#contactsBtn:pressed {"
        "    background-color: rgba(255,255,255,0.3);"
        "}"
    );
    connect(contactsBtn, &QtMaterialIconButton::clicked, this, [this]() {
        // 显示联系人功能的消息
        _snackbar->addMessage("联系人功能开发中");
    });
    
    QtMaterialIconButton *favoritesBtn = createNavButton(":/res/bookmark.png", "收藏");
    favoritesBtn->setObjectName("favoritesBtn");
    favoritesBtn->setStyleSheet(
        "QtMaterialIconButton#favoritesBtn {"
        "    color: white;"
        "    background-color: transparent;"
        "    border-radius: 18px;"
        "}"
        "QtMaterialIconButton#favoritesBtn:hover {"
        "    background-color: rgba(255,255,255,0.2);"
        "}"
        "QtMaterialIconButton#favoritesBtn:pressed {"
        "    background-color: rgba(255,255,255,0.3);"
        "}"
    );
    connect(favoritesBtn, &QtMaterialIconButton::clicked, this, [this]() {
        // 显示收藏功能的消息
        _snackbar->addMessage("收藏功能开发中");
    });
    
    // 底部设置按钮
    QtMaterialIconButton *settingsBtn = createNavButton(":/res/settings.png", "设置");
    settingsBtn->setObjectName("settingsBtn");
    settingsBtn->setStyleSheet(
        "QtMaterialIconButton#settingsBtn {"
        "    color: white;"
        "    background-color: transparent;"
        "    border-radius: 18px;"
        "}"
        "QtMaterialIconButton#settingsBtn:hover {"
        "    background-color: rgba(255,255,255,0.2);"
        "}"
        "QtMaterialIconButton#settingsBtn:pressed {"
        "    background-color: rgba(255,255,255,0.3);"
        "}"
    );
    connect(settingsBtn, &QtMaterialIconButton::clicked, this, [this]() {
        // 显示设置功能的消息
        _snackbar->addMessage("设置功能开发中");
    });
    
    // 添加到导航栏布局
    navBarLayout->addWidget(chatBtnContainer);
    navBarLayout->addWidget(contactsBtn);
    navBarLayout->addWidget(favoritesBtn);
    navBarLayout->addStretch(1);
    
    // 底部设置按钮
    navBarLayout->addWidget(settingsBtn);
    
    // =============== 中间聊天列表区 ===============
    QWidget *chatListWidget = new QWidget(contentContainer);
    chatListWidget->setMinimumWidth(280);
    chatListWidget->setMaximumWidth(300);
    chatListWidget->setStyleSheet("background-color: #F5F5F5;");
    
    QVBoxLayout *chatListLayout = new QVBoxLayout(chatListWidget);
    chatListLayout->setContentsMargins(0, 0, 0, 0);
    chatListLayout->setSpacing(0);
    
    // 搜索框
    QWidget *searchContainer = new QWidget(chatListWidget);
    searchContainer->setFixedHeight(50);
    searchContainer->setStyleSheet("background-color: #F5F5F5; border-bottom: 1px solid #E0E0E0;");
    
    QHBoxLayout *searchLayout = new QHBoxLayout(searchContainer);
    searchLayout->setContentsMargins(10, 10, 10, 10);
    
    QtMaterialTextField *searchBox = new QtMaterialTextField(searchContainer);
    searchBox->setPlaceholderText("搜索");
    searchBox->setShowLabel(false);
    searchBox->setInkColor(QColor("#2AABEE"));
    searchBox->setStyleSheet(
        "QtMaterialTextField {"
        "  border: none;"
        "  padding: 0;"
        "}"
        "QLineEdit {"
        "  border: none;"
        "  padding: 5px;"
        "  background: transparent;"
        "  font-size: 14px;"
        "}"
    );
    
    searchLayout->addWidget(searchBox);
    chatListLayout->addWidget(searchContainer);
    
    // 聊天导航选项
    QListWidget *navList = new QListWidget(chatListWidget);
    navList->setFrameShape(QFrame::NoFrame);
    navList->setMaximumHeight(80);
    navList->setStyleSheet(
        "QListWidget {"
        "  background-color: #F5F5F5;"
        "  border: none;"
        "}"
        "QListWidget::item {"
        "  padding: 10px 15px;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #E3F2FD;"
        "}"
    );
    
    QListWidgetItem *allChatsItem = new QListWidgetItem("所有聊天");
    navList->addItem(allChatsItem);
    
    QListWidgetItem *archivedItem = new QListWidgetItem("已归档");
    navList->addItem(archivedItem);
    
    chatListLayout->addWidget(navList);
    
    // 联系人/聊天列表
    _contactList = new QListWidget(chatListWidget);
    _contactList->setFrameShape(QFrame::NoFrame);
    _contactList->setStyleSheet(
        "QListWidget {"
        "  background-color: #F5F5F5;"
        "  border: none;"
        "}"
        "QListWidget::item {"
        "  padding: 10px 5px;"
        "  border-bottom: 1px solid #E0E0E0;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #E3F2FD;"
        "  border-left: 2px solid #2AABEE;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: #F5F5F5;"
        "}"
    );
    
    // 设置联系人列表的滚动条
    QtMaterialScrollBar *scrollBar = new QtMaterialScrollBar(_contactList);
    _contactList->setVerticalScrollBar(scrollBar);
    
    chatListLayout->addWidget(_contactList, 1); // 1表示会占用剩余空间
    
    // =============== 右侧聊天窗口 ===============
    QWidget *rightChatArea = new QWidget(contentContainer);
    QVBoxLayout *chatLayout = new QVBoxLayout(rightChatArea);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);
    
    // 聊天窗口标题栏
    QWidget *chatHeader = new QWidget(rightChatArea);
    chatHeader->setFixedHeight(50);
    chatHeader->setStyleSheet("background-color: #F5F5F5; border-bottom: 1px solid #E0E0E0;");
    
    QHBoxLayout *chatHeaderLayout = new QHBoxLayout(chatHeader);
    chatHeaderLayout->setContentsMargins(15, 0, 15, 0);
    
    QLabel *chatTitleLabel = new QLabel("联系人名称", chatHeader);
    chatTitleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    chatHeaderLayout->addWidget(chatTitleLabel);
    
    // 聊天记录区域
    _chatScrollArea = new QScrollArea(rightChatArea);
    _chatScrollArea->setWidgetResizable(true);
    _chatScrollArea->setFrameShape(QFrame::NoFrame);
    _chatScrollArea->setStyleSheet(
        "QScrollArea {"
        "  background-color: #E7EBF0;"
        "}"
    );
    
    _chatHistoryWidget = new QWidget(_chatScrollArea);
    _chatHistoryLayout = new QVBoxLayout(_chatHistoryWidget);
    _chatHistoryLayout->setAlignment(Qt::AlignTop);
    _chatHistoryLayout->setContentsMargins(10, 10, 10, 10);
    _chatHistoryWidget->setLayout(_chatHistoryLayout);
    
    _chatScrollArea->setWidget(_chatHistoryWidget);
    
    // 设置聊天滚动条
    QtMaterialScrollBar *chatScrollBar = new QtMaterialScrollBar(_chatScrollArea);
    _chatScrollArea->setVerticalScrollBar(chatScrollBar);
    
    // 输入区域
    QWidget *inputWidget = new QWidget(rightChatArea);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputWidget);
    inputLayout->setContentsMargins(15, 15, 15, 15);
    inputLayout->setSpacing(10);
    
    // 输入框容器
    QWidget *inputContainer = new QWidget(inputWidget);
    inputContainer->setObjectName("inputContainer");
    inputContainer->setStyleSheet(
        "QWidget#inputContainer {"
        "  background-color: white;"
        "  border-radius: 20px;"
        "  border: 1px solid #E0E0E0;"
        "}"
    );
    
    // 为输入容器添加阴影效果
    QGraphicsDropShadowEffect *inputShadow = new QGraphicsDropShadowEffect(inputContainer);
    inputShadow->setBlurRadius(15);
    inputShadow->setColor(QColor(0, 0, 0, 30));
    inputShadow->setOffset(0, 1);
    inputContainer->setGraphicsEffect(inputShadow);
    
    QHBoxLayout *containerLayout = new QHBoxLayout(inputContainer);
    containerLayout->setContentsMargins(15, 10, 15, 10);
    containerLayout->setSpacing(10);
    
    // 创建左侧按钮容器，确保垂直居中
    QWidget *leftButtonsContainer = new QWidget(inputContainer);
    QHBoxLayout *leftButtonsLayout = new QHBoxLayout(leftButtonsContainer);
    leftButtonsLayout->setContentsMargins(0, 0, 0, 0);
    leftButtonsLayout->setSpacing(5);
    leftButtonsLayout->setAlignment(Qt::AlignCenter); // 确保垂直居中对齐
    
    _emojiButton = new QtMaterialIconButton(QIcon(":/res/emoji.png"), leftButtonsContainer);
    _emojiButton->setIconSize(QSize(22, 22));
    _emojiButton->setFixedSize(32, 32);
    _emojiButton->setStyleSheet(
        "QtMaterialIconButton {"
        "    color: #757575;"
        "    background-color: transparent;"
        "    border-radius: 16px;"
        "}"
        "QtMaterialIconButton:hover {"
        "    background-color: rgba(0,0,0,0.05);"
        "}"
        "QtMaterialIconButton:pressed {"
        "    background-color: rgba(0,0,0,0.1);"
        "}"
    );
    
    _attachButton = new QtMaterialIconButton(QIcon(":/res/attach.png"), leftButtonsContainer);
    _attachButton->setIconSize(QSize(22, 22));
    _attachButton->setFixedSize(32, 32);
    _attachButton->setStyleSheet(
        "QtMaterialIconButton {"
        "    color: #757575;"
        "    background-color: transparent;"
        "    border-radius: 16px;"
        "}"
        "QtMaterialIconButton:hover {"
        "    background-color: rgba(0,0,0,0.05);"
        "}"
        "QtMaterialIconButton:pressed {"
        "    background-color: rgba(0,0,0,0.1);"
        "}"
    );
    
    leftButtonsLayout->addWidget(_emojiButton);
    leftButtonsLayout->addWidget(_attachButton);
    
    // 消息输入框
    _messageInput = new QtMaterialTextField(inputContainer);
    _messageInput->setLabel("输入消息");
    _messageInput->setInkColor(QColor("#2AABEE"));
    _messageInput->setShowLabel(false);
    _messageInput->setFixedHeight(40);
    _messageInput->setStyleSheet(
        "QtMaterialTextField {"
        "  border: none;"
        "  padding: 0;"
        "  margin-top: 3px;" // 微调垂直位置
        "}"
        "QLineEdit {"
        "  border: none;"
        "  padding: 8px 5px;"
        "  background: transparent;"
        "  font-size: 15px;"
        "}"
    );
    
    // 添加回车键发送功能
    connect(_messageInput, &QtMaterialTextField::returnPressed, this, &My_wechat::onSendButtonClicked);

    // 创建发送按钮容器以便居中对齐
    QWidget *sendButtonContainer = new QWidget(inputContainer);
    sendButtonContainer->setFixedSize(46, 46);
    QHBoxLayout *sendButtonLayout = new QHBoxLayout(sendButtonContainer);
    sendButtonLayout->setContentsMargins(0, 0, 0, 0);
    sendButtonLayout->setAlignment(Qt::AlignCenter);
    
    // 发送按钮 - 使用正确构造函数参数
    _sendButton = new QtMaterialFloatingActionButton(QIcon(":/res/send.png"), sendButtonContainer);
    _sendButton->setBackgroundColor(QColor("#2AABEE"));
    _sendButton->setFixedSize(42, 42);
    _sendButton->setUseThemeColors(false);
    _sendButton->setIconSize(QSize(24, 24)); // 增大图标尺寸，确保可见
    
    // 发送按钮的悬停和点击效果
    _sendButton->setStyleSheet(
        "QtMaterialFloatingActionButton {"
        "    border-radius: 21px;"
        "    background-color: #2AABEE;"
        "}"
        "QtMaterialFloatingActionButton:hover {"
        "    background-color: #1E88E5;"
        "}"
        "QtMaterialFloatingActionButton:pressed {"
        "    background-color: #1976D2;"
        "}"
    );
    
    sendButtonLayout->addWidget(_sendButton);
    
    // 将左侧按钮容器、输入框和发送按钮容器添加到布局
    containerLayout->addWidget(leftButtonsContainer);
    containerLayout->addWidget(_messageInput, 1);
    containerLayout->addWidget(sendButtonContainer);
    
    inputLayout->addWidget(inputContainer, 1);
    
    inputWidget->setLayout(inputLayout);
    inputWidget->setFixedHeight(90); // 调整高度使控件更协调
    inputWidget->setStyleSheet("background-color: #F5F5F5;");
    
    // 添加到聊天布局
    chatLayout->addWidget(chatHeader);
    chatLayout->addWidget(_chatScrollArea, 1);
    chatLayout->addWidget(inputWidget);
    
    // 将各个部分添加到内容布局
    contentLayout->addWidget(navBarWidget);
    contentLayout->addWidget(chatListWidget);
    contentLayout->addWidget(rightChatArea, 1); // 1表示占用剩余空间
    
    mainLayout->addWidget(contentContainer, 1);
    
    // Drawer等组件保持不变
    _drawer = new QtMaterialDrawer(this);
    _drawer->setClickOutsideToClose(true);
    _drawer->setOverlayMode(true);
    
    QVBoxLayout *drawerLayout = new QVBoxLayout;
    drawerLayout->setAlignment(Qt::AlignTop);
    
    // 创建抽屉头部
    QWidget *drawerHeader = new QWidget();
    drawerHeader->setFixedHeight(150);
    drawerHeader->setStyleSheet("background-color: #2AABEE;");
    
    QVBoxLayout *headerLayout = new QVBoxLayout(drawerHeader);
    headerLayout->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    headerLayout->setContentsMargins(16, 16, 16, 16);
    
    // 用户头像
    QtMaterialAvatar *userAvatar = new QtMaterialAvatar(QImage(":/res/default_avatar.png"), drawerHeader);
    userAvatar->setSize(54);
    
    // 用户名
    QLabel *usernameLabel = new QLabel("用户名");
    usernameLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    
    // 在线状态
    QLabel *statusLabel = new QLabel("在线");
    statusLabel->setStyleSheet("color: rgba(255,255,255,0.7); font-size: 13px;");
    
    headerLayout->addWidget(userAvatar);
    headerLayout->addSpacing(8);
    headerLayout->addWidget(usernameLabel);
    headerLayout->addWidget(statusLabel);
    
    drawerLayout->addWidget(drawerHeader);
    
    // 创建菜单项
    auto createMenuItem = [](const QString &text, const QString &iconPath) {
        QHBoxLayout *itemLayout = new QHBoxLayout();
        itemLayout->setContentsMargins(16, 12, 16, 12);
        
        QLabel *iconLabel = new QLabel();
        iconLabel->setPixmap(QPixmap(iconPath).scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        
        QLabel *textLabel = new QLabel(text);
        textLabel->setStyleSheet("font-size: 15px; color: #212121;");
        
        itemLayout->addWidget(iconLabel);
        itemLayout->addSpacing(32);
        itemLayout->addWidget(textLabel);
        itemLayout->addStretch();
        
        QWidget *item = new QWidget();
        item->setLayout(itemLayout);
        item->setCursor(Qt::PointingHandCursor);
        item->setStyleSheet(
            "QWidget:hover {"
            "    background-color: #F1F1F1;"
            "}"
        );
        
        return item;
    };
    
    // 添加菜单项
    drawerLayout->addWidget(createMenuItem("新建群组", ":/res/group.png"));
    drawerLayout->addWidget(createMenuItem("联系人", ":/res/contacts.png"));
    drawerLayout->addWidget(createMenuItem("保存的消息", ":/res/bookmark.png"));
    drawerLayout->addWidget(createMenuItem("设置", ":/res/settings.png"));
    drawerLayout->addStretch();
    
    QWidget *drawerWidget = new QWidget;
    drawerWidget->setLayout(drawerLayout);
    drawerWidget->setStyleSheet("background-color: white;");
    
    _drawer->setDrawerLayout(drawerLayout);
    _drawer->setDrawerWidth(280);
    
    // 创建 Snackbar
    _snackbar = new QtMaterialSnackbar(this);
    
    // 添加联系人项目
    addContactItem("张三", "你好，最近怎么样？", "");
    addContactItem("李四", "周末有空吗？", "");
    addContactItem("王五", "项目进展如何？", "");
    addContactItem("赵六", "记得带上文件！", "");
    
    // 连接信号和槽
    connect(_sendButton, &QtMaterialFloatingActionButton::clicked, this, &My_wechat::onSendButtonClicked);
    connect(_contactList, &QListWidget::currentRowChanged, this, &My_wechat::contactSelected);
    connect(_emojiButton, &QtMaterialIconButton::clicked, this, &My_wechat::openEmoji);
    connect(_attachButton, &QtMaterialIconButton::clicked, this, &My_wechat::attachFile);
    
    // 如果联系人列表不为空，则选择第一个联系人
    if (_contactList->count() > 0) {
        _contactList->setCurrentRow(0);
        contactSelected(0);
    }
    
    // 底部状态栏
    QWidget *bottomBar = new QWidget(rightChatArea);
    bottomBar->setFixedHeight(40);
    bottomBar->setStyleSheet("background-color: #F5F5F5; border-top: 1px solid #E0E0E0;");
    
    QHBoxLayout *bottomBarLayout = new QHBoxLayout(bottomBar);
    bottomBarLayout->setContentsMargins(15, 0, 15, 0);
    
    QLabel *contactsLabel = new QLabel("联系人", bottomBar);
    contactsLabel->setStyleSheet("color: #2AABEE;");
    
    QLabel *settingsLabel = new QLabel("设置", bottomBar);
    settingsLabel->setStyleSheet("color: #2AABEE;");
    
    bottomBarLayout->addWidget(contactsLabel);
    bottomBarLayout->addStretch();
    bottomBarLayout->addWidget(settingsLabel);
    
    chatLayout->addWidget(bottomBar);
}

void My_wechat::addContactItem(const QString &name, const QString &lastMsg, const QString &avatarPath)
{
    QListWidgetItem *item = new QListWidgetItem(_contactList);
    ContactItem *contactItem = new ContactItem(name, lastMsg, avatarPath);
    
    item->setSizeHint(contactItem->sizeHint());
    _contactList->addItem(item);
    _contactList->setItemWidget(item, contactItem);
}

void My_wechat::addMessageBubble(const QString &message, ChatBubble::BubbleType type)
{
    ChatBubble *bubble = new ChatBubble(message, type);
    _chatHistoryLayout->addWidget(bubble);
    
    // 延迟加载更多消息
    QTimer::singleShot(100, [this]() {
        _chatScrollArea->verticalScrollBar()->setValue(
            _chatScrollArea->verticalScrollBar()->maximum()
        );
    });
}

void My_wechat::onSendButtonClicked()
{
    QString message = _messageInput->text().trimmed();
    if (!message.isEmpty()) {
        // 发送消息
        sendMessage(message);
        
        // 清空输入框
        _messageInput->clear();
    }
}

void My_wechat::sendMessage(const QString &message)
{
    // 添加自己发送的消息气泡
    addMessageBubble(message, ChatBubble::Sent);
        
        // 模拟回复
    QTimer::singleShot(1000, [this, message]() {
            QString reply;
        if (message.contains("你好") || message.contains("嗨") || message.contains("hi")) {
                reply = "你好！很高兴见到你，有什么可以帮助你的吗？";
        } else if (message.contains("天气") || message.contains("下雨")) {
                reply = "今天天气不错，适合出去走走";
        } else if (message.contains("?") || message.contains("？")) {
                reply = "这是一个好问题，让我想想怎么回答比较好...";
            } else {
                QStringList responses = {
                    "这个想法很有趣，能说得更具体一些吗？",
                    "明白了",
                    "这确实值得好好考虑",
                    "我觉得这个建议很不错，我们可以继续讨论",
                    "嗯，这个话题很有意思，能展开说说吗？"
                };
                reply = responses.at(QRandomGenerator::global()->bounded(responses.size()));
            }
            addMessageBubble(reply, ChatBubble::Received);
        });
}

void My_wechat::contactSelected(int index)
{
    if (index >= 0) {
        QListWidgetItem *item = _contactList->item(index);
        QWidget *widget = _contactList->itemWidget(item);
        
        // 清除聊天历史
        while (QLayoutItem *layoutItem = _chatHistoryLayout->takeAt(0)) {
            delete layoutItem->widget();
            delete layoutItem;
        }
        
        // 获取联系人信息
        QList<QLabel*> labels = widget->findChildren<QLabel*>();
        if (!labels.isEmpty()) {
            _currentContact = labels.first()->text();
            
            // 更新标题栏
            QList<QLabel*> titleLabels = _appBar->findChildren<QLabel*>();
            if (!titleLabels.isEmpty()) {
                titleLabels.first()->setText(_currentContact);
            }
            
            // 添加联系人消息
            if (_currentContact == "张三") {
                addMessageBubble("你好，最近怎么样？工作顺利吗？", ChatBubble::Received);
                addMessageBubble("挺好的，就是最近项目比较忙", ChatBubble::Sent);
                addMessageBubble("忙点好，说明公司发展不错。有空一起吃个饭？", ChatBubble::Received);
            } else if (_currentContact == "李四") {
                addMessageBubble("周末有空吗？一起打球去！", ChatBubble::Received);
                addMessageBubble("好啊，周六见", ChatBubble::Sent);
                addMessageBubble("下午2点，老地方集合", ChatBubble::Received);
            } else if (_currentContact == "王五") {
                addMessageBubble("最近项目进展如何？需要帮忙吗？", ChatBubble::Sent);
                addMessageBubble("进展还不错", ChatBubble::Received);
                addMessageBubble("项目文档已经完成了", ChatBubble::Received);
            } else if (_currentContact == "赵六") {
                addMessageBubble("明天开会别忘了带文件", ChatBubble::Received);
                addMessageBubble("好的，我已经准备好了", ChatBubble::Sent);
            }
        }
    }
}

void My_wechat::openEmoji()
{
    // 打开表情选择器
    _snackbar->addMessage("表情面板功能开发中");
}

void My_wechat::attachFile()
{
    // 打开文件选择器
    _snackbar->addMessage("附件功能开发中");
}

void My_wechat::showDrawer()
{
    _drawer->openDrawer();
    _isDrawerOpen = true;
}

void My_wechat::toggleDrawer()
{
    if (_isDrawerOpen) {
        _drawer->closeDrawer();
        _isDrawerOpen = false;
    } else {
        _drawer->openDrawer();
        _isDrawerOpen = true;
    }
}

void My_wechat::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->pos().y() < _appBar->height()) {
        _windowMoving = true;
        _dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void My_wechat::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        _windowMoving = false;
        event->accept();
    }
}

void My_wechat::mouseMoveEvent(QMouseEvent *event)
{
    if (_windowMoving) {
        move(event->globalPosition().toPoint() - _dragPosition);
        event->accept();
    }
}

void My_wechat::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    
    // 在窗口大小变化时更新布局
    updateLayout();
}

void My_wechat::updateLayout()
{
    // 确保聊天记录能够滚动到最底部
    if (_chatScrollArea && _chatScrollArea->verticalScrollBar()) {
        _chatScrollArea->verticalScrollBar()->setValue(
            _chatScrollArea->verticalScrollBar()->maximum()
        );
    }
}

My_wechat::~My_wechat()
{
    // 不需要再删除 _login_dialog 和 _reg_dlg，因为它们已经由 AppController 管理
}

// 添加加载用户信息的方法
void My_wechat::loadUserInfo()
{
    // 检查用户是否已登录
    if (UserManager::GetInstance()->isLoggedIn())
    {
        // 获取当前用户信息
        const UserInfo& user = UserManager::GetInstance()->getCurrentUser();
        
        // 设置用户昵称到界面
        QLabel* titleLabel = findChild<QLabel*>("titleLabel");
        if (titleLabel) {
            titleLabel->setText(QString("微信 - %1").arg(user.nickname));
        }
        
        // 这里可以添加更多用户信息的展示，例如头像等
        // TODO: 显示用户头像和其他信息
    }
}
