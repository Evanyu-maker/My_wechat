#include "ChatBubble.h"

ChatBubble::ChatBubble(const QString &text, BubbleType type, QWidget *parent)
    : QWidget(parent), bubbleType(type)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(2);
    
    QHBoxLayout *bubbleLayout = new QHBoxLayout();
    bubbleLayout->setSpacing(0);
    
    // 创建消息标签
    messageLabel = new QLabel(text);
    messageLabel->setWordWrap(true);
    messageLabel->setMaximumWidth(400);
    messageLabel->setMinimumHeight(30);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    // 根据消息类型设置不同的样式
    if (type == Sent) {
        messageLabel->setStyleSheet(
            "QLabel {"
            "  background-color: #95EC69;"
            "  border-radius: 10px;"
            "  padding: 10px;"
            "  margin-right: 5px;"
            "  font-size: 14px;"
            "}"
        );
        bubbleLayout->addStretch();
        bubbleLayout->addWidget(messageLabel);
    } else {
        messageLabel->setStyleSheet(
            "QLabel {"
            "  background-color: #FFFFFF;"
            "  border-radius: 10px;"
            "  padding: 10px;"
            "  margin-left: 5px;"
            "  font-size: 14px;"
            "}"
        );
        bubbleLayout->addWidget(messageLabel);
        bubbleLayout->addStretch();
    }
    
    // 创建时间标签
    timeLabel = new QLabel(QDateTime::currentDateTime().toString("hh:mm"));
    timeLabel->setStyleSheet("color: #8C8C8C; font-size: 12px;");
    
    QHBoxLayout *timeLayout = new QHBoxLayout();
    if (type == Sent) {
        timeLayout->addStretch();
        timeLayout->addWidget(timeLabel);
    } else {
        timeLayout->addWidget(timeLabel);
        timeLayout->addStretch();
    }
    
    mainLayout->addLayout(bubbleLayout);
    mainLayout->addLayout(timeLayout);
    
    setLayout(mainLayout);
} 