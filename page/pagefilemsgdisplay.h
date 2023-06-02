#ifndef PAGEFILEMSGDISPLAY_H
#define PAGEFILEMSGDISPLAY_H

#include <QWidget>
#include "pagemsgdisplay.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QtNetwork>

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

    void parse(uint32_t pgn, QVector<uint8_t> data);

private:
    void msg_data_polt_send(MsgData *msg_data);
    void write_csv_data(MsgData *msg_data);

private slots:
    void csv_save_file();

private:
    uint8_t src;

    Ui::PageFileMsgDisplay         *ui;
    QMap<QString, PageMsgDisplay *> msg_page_map;

    QUdpSocket *udp_socket;

    QTimer csv_timer;

    QStringList            csv_header;
    QString                csv_time_key;
    QString                csv_path;
    QMap<QString, QString> csv_map;
};

#endif // PAGEFILEMSGDISPLAY_H
