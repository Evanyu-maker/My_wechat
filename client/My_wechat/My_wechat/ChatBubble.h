#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>

class ChatBubble : public QWidget
{
    Q_OBJECT

public:
    enum BubbleType { Sent, Received };

    ChatBubble(const QString &text, BubbleType type, QWidget *parent = nullptr);
    
private:
    QLabel *messageLabel;
    QLabel *timeLabel;
    BubbleType bubbleType;
}; 