#include "My_wechat.h"
#include "ui_My_wechat.h"
#include "ChatBubble.h"
#include "ContactItem.h"
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

My_wechat::My_wechat(QWidget *parent)
    : QMainWindow(parent), _isMousePressed(false)
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
}

void My_wechat::setupChatUi()
{
    setWindowTitle("微信聊天");
    resize(950, 1000);
    setMinimumSize(950, 1000);
    
    // 创建主窗口部件
    _centralWidget = new QWidget(this);
    setCentralWidget(_centralWidget);
    
    // 创建 Material Design 风格的顶部栏
    _appBar = new QtMaterialAppBar(this);
    _appBar->setBackgroundColor(QColor("#07C160"));
    _appBar->setFixedHeight(64);
    
    QLabel *titleLabel = new QLabel("微信");
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
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
            maxButton->setIcon(QIcon(":/res/Max_.png"));
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
    
    QVBoxLayout *mainLayout = new QVBoxLayout(_centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(_appBar);
    
    // 创建分割器
    _splitter = new QSplitter(Qt::Horizontal, _centralWidget);
    
    // 创建左侧面板
    QWidget *leftPanel = new QWidget(_splitter);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    
    _contactList = new QListWidget(leftPanel);
    _contactList->setFrameShape(QFrame::NoFrame);
    _contactList->setStyleSheet(
        "QListWidget {"
        "  background-color: #F5F5F5;"
        "  border: none;"
        "}"
        "QListWidget::item {"
        "  padding: 0px;"
        "  border: none;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #E0E0E0;"
        "  border: none;"
        "}"
    );
    
    // 设置左侧面板的滚动条
    QtMaterialScrollBar *scrollBar = new QtMaterialScrollBar(_contactList);
    _contactList->setVerticalScrollBar(scrollBar);
    
    leftLayout->addWidget(_contactList);
    leftPanel->setLayout(leftLayout);
    
    // 创建聊天窗口部件
    _chatWidget = new QWidget(_splitter);
    QVBoxLayout *chatLayout = new QVBoxLayout(_chatWidget);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);
    
    // 创建聊天滚动区域
    _chatScrollArea = new QScrollArea(_chatWidget);
    _chatScrollArea->setWidgetResizable(true);
    _chatScrollArea->setFrameShape(QFrame::NoFrame);
    _chatScrollArea->setStyleSheet("background-color: #ECECEC;");
    
    _chatHistoryWidget = new QWidget(_chatScrollArea);
    _chatHistoryLayout = new QVBoxLayout(_chatHistoryWidget);
    _chatHistoryLayout->setAlignment(Qt::AlignTop);
    _chatHistoryLayout->setContentsMargins(10, 10, 10, 10);
    _chatHistoryWidget->setLayout(_chatHistoryLayout);
    
    _chatScrollArea->setWidget(_chatHistoryWidget);
    
    // 设置聊天滚动条
    QtMaterialScrollBar *chatScrollBar = new QtMaterialScrollBar(_chatScrollArea);
    _chatScrollArea->setVerticalScrollBar(chatScrollBar);
    
    // 创建输入部件
    QWidget *inputWidget = new QWidget(_chatWidget);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputWidget);
    inputLayout->setContentsMargins(10, 10, 10, 10);
    
    _emojiButton = new QtMaterialIconButton(QIcon(":/res/emoji.png"), inputWidget);
    _attachButton = new QtMaterialIconButton(QIcon(":/res/attach.png"), inputWidget);
    _attachButton->setIconSize(QSize(24, 24));
    
    _messageInput = new QtMaterialTextField(inputWidget);
    _messageInput->setLabel("输入消息");
    _messageInput->setInkColor(QColor("#07C160"));
    _messageInput->setShowLabel(false);
    
    _sendButton = new QtMaterialRaisedButton("发送", inputWidget);
    _sendButton->setBackgroundColor(QColor("#07C160"));
    _sendButton->setFixedWidth(80);
    
    inputLayout->addWidget(_emojiButton);
    inputLayout->addWidget(_attachButton);
    inputLayout->addWidget(_messageInput, 1);
    inputLayout->addWidget(_sendButton);
    
    inputWidget->setLayout(inputLayout);
    inputWidget->setFixedHeight(70);
    inputWidget->setStyleSheet("background-color: white;");
    
    chatLayout->addWidget(_chatScrollArea, 1);
    chatLayout->addWidget(inputWidget);
    
    _chatWidget->setLayout(chatLayout);
    
    // 设置聊天窗口部件
    _splitter->addWidget(leftPanel);
    _splitter->addWidget(_chatWidget);
    _splitter->setStretchFactor(0, 1);
    _splitter->setStretchFactor(1, 3);
    
    mainLayout->addWidget(_splitter);
    
    // 创建抽屉
    _drawer = new QtMaterialDrawer(this);
    _drawer->setClickOutsideToClose(true);
    _drawer->setOverlayMode(true);
    
    QVBoxLayout *drawerLayout = new QVBoxLayout;
    drawerLayout->setAlignment(Qt::AlignTop);
    
    QLabel *drawerTitle = new QLabel("设置");
    drawerTitle->setStyleSheet("font-size: 24px; padding: 20px;");
    
    QtMaterialFlatButton *profileBtn = new QtMaterialFlatButton("个人资料");
    QtMaterialFlatButton *settingsBtn = new QtMaterialFlatButton("设置");
    QtMaterialFlatButton *notificationsBtn = new QtMaterialFlatButton("消息通知");
    QtMaterialFlatButton *aboutBtn = new QtMaterialFlatButton("关于");
    
    profileBtn->setFixedWidth(200);
    settingsBtn->setFixedWidth(200);
    notificationsBtn->setFixedWidth(200);
    aboutBtn->setFixedWidth(200);
    
    drawerLayout->addWidget(drawerTitle);
    drawerLayout->addWidget(profileBtn);
    drawerLayout->addWidget(settingsBtn);
    drawerLayout->addWidget(notificationsBtn);
    drawerLayout->addWidget(aboutBtn);
    
    QWidget *drawerWidget = new QWidget;
    drawerWidget->setLayout(drawerLayout);
    drawerWidget->setStyleSheet("background-color: white;");
    
    _drawer->setDrawerLayout(drawerLayout);
    _drawer->setDrawerWidth(250);
    
    // 创建浮动按钮
    _newChatButton = new QtMaterialFloatingActionButton(QIcon(":/res/add.png"), this);
    _newChatButton->setBackgroundColor(QColor("#07C160"));
    _newChatButton->setCorner(Qt::BottomRightCorner);
    _newChatButton->setOffset(30, 30);
    
    // 创建 Snackbar
    _snackbar = new QtMaterialSnackbar(this);
    
    // 添加联系人项目
    addContactItem("张三", "你好，最近怎么样？", "");
    addContactItem("李四", "周末有空吗？", "");
    addContactItem("王五", "项目进展如何？", "");
    addContactItem("赵六", "记得带上文件！", "");
    
    // 连接信号和槽
    connect(_sendButton, &QtMaterialRaisedButton::clicked, this, &My_wechat::sendMessage);
    connect(_contactList, &QListWidget::currentRowChanged, this, &My_wechat::contactSelected);
    connect(_emojiButton, &QtMaterialIconButton::clicked, this, &My_wechat::openEmoji);
    connect(_attachButton, &QtMaterialIconButton::clicked, this, &My_wechat::attachFile);
    connect(_newChatButton, &QtMaterialFloatingActionButton::clicked, this, &My_wechat::showDrawer);
    
    // 如果联系人列表不为空，则选择第一个联系人
    if (_contactList->count() > 0) {
        _contactList->setCurrentRow(0);
        contactSelected(0);
    }
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

void My_wechat::sendMessage()
{
    QString text = _messageInput->text().trimmed();
    if (!text.isEmpty()) {
        addMessageBubble(text, ChatBubble::Sent);
        _messageInput->clear();
        
        // 模拟回复
        QTimer::singleShot(1000, [this, text]() {
            QString reply;
            if (text.contains("你好") || text.contains("嗨") || text.contains("hi")) {
                reply = "你好！很高兴见到你，有什么可以帮助你的吗？";
            } else if (text.contains("天气") || text.contains("下雨")) {
                reply = "今天天气不错，适合出去走走";
            } else if (text.contains("?") || text.contains("？")) {
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
}

void My_wechat::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->pos().y() < _appBar->height()) {
        _isMousePressed = true;
        _mousePos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void My_wechat::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        _isMousePressed = false;
        event->accept();
    }
}

void My_wechat::mouseMoveEvent(QMouseEvent *event)
{
    if (_isMousePressed) {
        move(event->globalPosition().toPoint() - _mousePos);
        event->accept();
    }
}

My_wechat::~My_wechat()
{
    // 不需要再删除 _login_dialog 和 _reg_dlg，因为它们已经由 AppController 管理
}
