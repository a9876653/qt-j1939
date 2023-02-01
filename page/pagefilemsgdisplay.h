#ifndef PAGEFILEMSGDISPLAY_H
#define PAGEFILEMSGDISPLAY_H

#include <QWidget>
#include "pagemsgdisplay.h"
#include "pageparams.h"

namespace Ui
{
class PageFileMsgDisplay;
}

class PageFileMsgDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit PageFileMsgDisplay(uint8_t src);
    ~PageFileMsgDisplay();

    void parse(uint32_t pgn, uint8_t *data, uint16_t len);

private:
    uint8_t src;

    Ui::PageFileMsgDisplay         *ui;
    QMap<QString, PageMsgDisplay *> msg_page_map;
    QMap<QString, PageParams *>     page_params_map;
};

#endif // PAGEFILEMSGDISPLAY_H
