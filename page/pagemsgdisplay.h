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
    explicit PageMsgDisplay(QMap<uint, MsgData> msgs_map);
    ~PageMsgDisplay();

    QMap<uint, MsgData> msgs_map;

private:
    Ui::PageMsgDisplay *ui;
};

#endif // PAGEMSGDISPLAY_H
