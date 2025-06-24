#include "ContactItem.h"
#include "qtmaterialavatar.h"
#include <QDateTime>

ContactItem::ContactItem(const QString &name, const QString &lastMsg, const QString &avatarPath, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 8, 16, 8);  // 增加边距
    layout->setSpacing(12);  // 增加间距
    
    // 创建头像
    avatar = new QtMaterialAvatar(this);
    avatar->setSize(50);  // 增大头像尺寸
    
    if (!avatarPath.isEmpty()) {
        avatar->setImage(QImage(avatarPath));
    } else {
        // 为不同联系人设置不同颜色的头像
        QColor avatarColor;
        int nameHash = qHash(name) % 7;  // 根据名字生成0-6的数字
        
        switch (nameHash) {
            case 0: avatarColor = QColor("#F44336"); break;  // 红色
            case 1: avatarColor = QColor("#2196F3"); break;  // 蓝色
            case 2: avatarColor = QColor("#4CAF50"); break;  // 绿色
            case 3: avatarColor = QColor("#FF9800"); break;  // 橙色
            case 4: avatarColor = QColor("#9C27B0"); break;  // 紫色
            case 5: avatarColor = QColor("#00BCD4"); break;  // 青色
            case 6: avatarColor = QColor("#607D8B"); break;  // 蓝灰色
            default: avatarColor = QColor("#2AABEE"); break; // 默认Telegram蓝
        }
        
        avatar->setBackgroundColor(avatarColor);
        avatar->setLetter(QChar(name.at(0).toUpper()));  // 确保是大写字母
    }
    
    // 创建名称和最后消息的布局
    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);  // 减小行间距
    textLayout->setContentsMargins(0, 4, 0, 4);  // 增加上下内边距
    
    nameLabel = new QLabel(name);
    nameLabel->setStyleSheet("font-weight: 500; font-size: 15px; color: #212121;");
    
    lastMessageLabel = new ElideLabel(lastMsg);
    lastMessageLabel->setStyleSheet("color: #8C8C8C; font-size: 13px;");
    lastMessageLabel->setMaximumWidth(250);  // 限制宽度
    lastMessageLabel->setElideMode(Qt::ElideRight);  // 省略号在右侧
    
    textLayout->addWidget(nameLabel);
    textLayout->addWidget(lastMessageLabel);
    
    // 右侧垂直布局（时间+未读计数）
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(2);
    rightLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);
    
    // 创建时间标签
    timeLabel = new QLabel(QDateTime::currentDateTime().toString("HH:mm"));
    timeLabel->setStyleSheet("color: #8C8C8C; font-size: 12px;");
    
    // 添加一个未读消息计数标签（如果需要的话）
    QLabel *unreadLabel = new QLabel("2");  // 示例数字，实际使用时应根据未读消息数设置
    unreadLabel->setFixedSize(22, 22);
    unreadLabel->setAlignment(Qt::AlignCenter);
    unreadLabel->setStyleSheet(
        "background-color: #2AABEE;"
        "color: white;"
        "border-radius: 11px;"
        "font-size: 11px;"
        "font-weight: bold;"
    );
    
    // 只在有未读消息时显示
    bool hasUnread = (qHash(name) % 3 == 0);  // 仅作为示例，随机显示未读计数
    unreadLabel->setVisible(hasUnread);
    
    rightLayout->addWidget(timeLabel);
    rightLayout->addWidget(unreadLabel, 0, Qt::AlignRight);
    
    layout->addWidget(avatar);
    layout->addLayout(textLayout, 1);
    layout->addLayout(rightLayout);
    
    setLayout(layout);
    setFixedHeight(72);  // 增加高度
    
    // 设置悬停和选中效果
    setStyleSheet(
        "ContactItem {"
        "  border-radius: 0px;"
        "}"
        "ContactItem:hover {"
        "  background-color: #F5F5F5;"
        "}"
    );
} 