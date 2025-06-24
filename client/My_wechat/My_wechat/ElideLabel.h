#pragma once

#include <QLabel>
#include <QPainter>
#include <QResizeEvent>

class ElideLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode)

public:
    explicit ElideLabel(QWidget *parent = nullptr);
    explicit ElideLabel(const QString &text, QWidget *parent = nullptr);
    
    Qt::TextElideMode elideMode() const;
    void setElideMode(Qt::TextElideMode mode);
    
    void setText(const QString &text);
    QString fullText() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateElided();
    
    QString m_fullText;
    QString m_elidedText;
    Qt::TextElideMode m_elideMode;
}; 