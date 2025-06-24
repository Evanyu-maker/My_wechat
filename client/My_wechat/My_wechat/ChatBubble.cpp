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
    messageLabel->setMaximumWidth(500);
    messageLabel->setMinimumHeight(30);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    // 根据消息类型设置不同的样式
    if (type == Sent) {
        messageLabel->setStyleSheet(
            "QLabel {"
            "  background-color: #E3F2FD;"
            "  color: #000000;"
            "  border-radius: 18px;"
            "  border-bottom-right-radius: 4px;"
            "  padding: 12px;"
            "  margin-right: 5px;"
            "  font-size: 14px;"
            "  font-family: 'Segoe UI', Arial;"
            "}"
        );
        bubbleLayout->addStretch();
        bubbleLayout->addWidget(messageLabel);
    } else {
        messageLabel->setStyleSheet(
            "QLabel {"
            "  background-color: #FFFFFF;"
            "  color: #000000;"
            "  border-radius: 18px;"
            "  border-bottom-left-radius: 4px;"
            "  padding: 12px;"
            "  margin-left: 5px;"
            "  font-size: 14px;"
            "  font-family: 'Segoe UI', Arial;"
            "  box-shadow: 0 1px 2px rgba(0,0,0,0.1);"
            "}"
        );
        bubbleLayout->addWidget(messageLabel);
        bubbleLayout->addStretch();
    }
    
    // 创建时间标签
    timeLabel = new QLabel(QDateTime::currentDateTime().toString("HH:mm"));
    timeLabel->setStyleSheet("color: #8C8C8C; font-size: 11px; margin-top: -2px;");
    
    QHBoxLayout *timeLayout = new QHBoxLayout();
    if (type == Sent) {
        timeLayout->addStretch();
        timeLayout->addWidget(timeLabel);
        timeLayout->setContentsMargins(0, 0, 15, 0);
    } else {
        timeLayout->addWidget(timeLabel);
        timeLayout->addStretch();
        timeLayout->setContentsMargins(15, 0, 0, 0);
    }
    
    mainLayout->addLayout(bubbleLayout);
    mainLayout->addLayout(timeLayout);
    
    setLayout(mainLayout);
} 