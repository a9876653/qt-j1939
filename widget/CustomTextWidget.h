#pragma once

#include <QTextEdit>
#include "QKeyEvent"

class CustomTextWidget : public QObject
{
    Q_OBJECT
public:
    CustomTextWidget()
    {
        text = new QTextEdit;
        // text->setFrameShape(QFrame::NoFrame);                     // 无边框
        text->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //无滚动条
        text->installEventFilter(this);                           // 设置事件过滤器
        text->setFixedHeight(25);                                 // 设置高度
        text->setFixedWidth(80);                                  // 设置高度
    }
    QTextEdit *text = nullptr;
signals:
    void sig_enter_event();

private:
    bool eventFilter(QObject *target, QEvent *event)
    {
        if (target == text)
        {
            if (event->type() == QEvent::KeyPress) // 按键按下
            {
                QKeyEvent *k = static_cast<QKeyEvent *>(event);
                if (k->key() == Qt::Key_Enter) // 按下回车键
                {
                    emit sig_enter_event(); // 触发事件
                    return true;
                }
            }
        }
        return false;
    }
};
