#ifndef WIDGETNUMLINEEDIT_H
#define WIDGETNUMLINEEDIT_H

#include <QLineEdit>
#include "QKeyEvent"

class WidgetNumLineEdit : public QObject
{
    Q_OBJECT
public:
    WidgetNumLineEdit(QString s = "");

    QLineEdit *line_edit = nullptr;

signals:
    void sig_enter_event();

private:
    bool eventFilter(QObject *target, QEvent *event);
};

#endif // WIDGETNUMLINEEDIT_H
