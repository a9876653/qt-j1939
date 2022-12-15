#ifndef PAGEMSGDISPLAY_H
#define PAGEMSGDISPLAY_H

#include <QWidget>
#include "msg_signals.h"

namespace Ui
{
class PageMsgDisplay;
}

class PageMsgDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit PageMsgDisplay(QMap<uint, MsgData *> msgs_map);
    ~PageMsgDisplay();

    QMap<uint, MsgData *> msgs_map;

private slots:
    void on_expandPushButton_clicked();

    void on_collapsPpushButton_clicked();

private:
    Ui::PageMsgDisplay *ui;
};

#endif // PAGEMSGDISPLAY_H
