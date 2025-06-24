#include "ElideLabel.h"
#include <QFontMetrics>

ElideLabel::ElideLabel(QWidget *parent)
    : QLabel(parent)
    , m_elideMode(Qt::ElideRight)
{
}

ElideLabel::ElideLabel(const QString &text, QWidget *parent)
    : QLabel(parent)
    , m_fullText(text)
    , m_elideMode(Qt::ElideRight)
{
    updateElided();
    QLabel::setText(m_elidedText);
}

void ElideLabel::setText(const QString &text)
{
    m_fullText = text;
    updateElided();
    QLabel::setText(m_elidedText);
}

QString ElideLabel::fullText() const
{
    return m_fullText;
}

Qt::TextElideMode ElideLabel::elideMode() const
{
    return m_elideMode;
}

void ElideLabel::setElideMode(Qt::TextElideMode mode)
{
    if (m_elideMode != mode) {
        m_elideMode = mode;
        updateElided();
        update();
    }
}

void ElideLabel::updateElided()
{
    QFontMetrics metrics(font());
    m_elidedText = metrics.elidedText(m_fullText, m_elideMode, width());
}

void ElideLabel::paintEvent(QPaintEvent *event)
{
    if (m_elideMode == Qt::ElideNone) {
        QLabel::paintEvent(event);
        return;
    }

    // 用自定义绘制替代默认的绘制行为
    QPainter painter(this);
    QRect r = rect();
    painter.setPen(palette().color(QPalette::WindowText));
    painter.drawText(r, alignment(), m_elidedText);
}

void ElideLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    
    // 每当大小改变，都需要重新计算省略文本
    updateElided();
    QLabel::setText(m_elidedText);
} 