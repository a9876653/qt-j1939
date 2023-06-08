#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "msg_signals.h"
#include "pagefilemsgdisplay.h"
#include "pageparse.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_openDevicePushButton_clicked();

    void slot_recv_pgn_handle(uint32_t pgn, uint8_t src, QVector<uint8_t> data);

    void on_srcAddrSpinBox_valueChanged(int arg1);

    void on_objAddrspinBox_valueChanged(int arg1);

    void slot_recv_comm_status(int ret);

    void on_consoleBtn_clicked();

private:
    Ui::MainWindow *ui;

    bool device_status = false;

    MsgSignals *msgs = nullptr;

    PageParse *src_page_parse = nullptr;

    bool console_is_show = false;

    class Pages
    {
    public:
        PageFileMsgDisplay *can_dbc    = nullptr;
        PageParse          *can_modbus = nullptr;
    };
    QMap<uint, Pages *> src_page_map;

    void save_cfg_data(QString path);
    void load_cfg_data(QString path);
};
#endif // MAINWINDOW_H
