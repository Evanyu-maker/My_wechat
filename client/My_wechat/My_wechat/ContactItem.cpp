#include "ContactItem.h"
#include "qtmaterialavatar.h"
#include <QDateTime>

ContactItem::ContactItem(const QString &name, const QString &lastMsg, const QString &avatarPath, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);
    
    // 创建头像
    avatar = new QtMaterialAvatar(this);
    avatar->setSize(40);
    if (!avatarPath.isEmpty()) {
        avatar->setImage(QImage(avatarPath));
    } else {
        avatar->setLetter(QChar(name.at(0)));
    }
    
    // 创建名称和最后消息的布局
    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setSpacing(0);
    
    nameLabel = new QLabel(name);
    nameLabel->setStyleSheet("font-weight: bold; font-size: 15px;");
    
    lastMessageLabel = new QLabel(lastMsg);
    lastMessageLabel->setStyleSheet("color: #8C8C8C; font-size: 13px;");
    
    textLayout->addWidget(nameLabel);
    textLayout->addWidget(lastMessageLabel);
    
    // 创建时间标签
    timeLabel = new QLabel(QDateTime::currentDateTime().toString("hh:mm"));
    timeLabel->setStyleSheet("color: #8C8C8C; font-size: 12px;");
    
    layout->addWidget(avatar);
    layout->addLayout(textLayout, 1);
    layout->addWidget(timeLabel);
    
    setLayout(layout);
    setFixedHeight(60);
    
    // 设置悬停效果
    setStyleSheet(
        "ContactItem:hover {"
        "  background-color: #F0F0F0;"
        "}"
    );
} 