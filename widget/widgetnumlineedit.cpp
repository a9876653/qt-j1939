#include "widgetnumlineedit.h"
#include "QObject"
#include "QRegExpValidator"
#include "QFontMetrics"

WidgetNumLineEdit::WidgetNumLineEdit(QString s)
{
    QRegExp exp("[0-9\\.-]+$");

    line_edit = new QLineEdit(s);
    line_edit->setValidator(new QRegExpValidator(exp));
    line_edit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    line_edit->installEventFilter(this); // 设置事件过滤器
}

bool WidgetNumLineEdit::eventFilter(QObject *target, QEvent *event)
{
    if (target == line_edit)
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
