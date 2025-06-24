#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "qtmaterialavatar.h"
#include "ElideLabel.h"

class ContactItem : public QWidget
{
    Q_OBJECT

public:
    ContactItem(const QString &name, const QString &lastMsg, const QString &avatarPath, QWidget *parent = nullptr);

private:
    QtMaterialAvatar *avatar;
    QLabel *nameLabel;
    ElideLabel *lastMessageLabel;
    QLabel *timeLabel;
}; 