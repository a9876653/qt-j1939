#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "msg_signals.h"
#include "pagefilemsgdisplay.h"

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

    void slot_recv_pgn_handle(uint32_t pgn, uint8_t src, uint8_t *data, uint16_t data_size);

    void on_srcAddrSpinBox_valueChanged(int arg1);

    void on_objAddrspinBox_valueChanged(int arg1);

    void slot_recv_comm_status(int ret);

private:
    Ui::MainWindow *ui;

    bool device_status = false;

    MsgSignals *msgs = nullptr;

    QMap<uint, PageFileMsgDisplay *> src_page_map;

    void save_cfg_data(QString path);
    void load_cfg_data(QString path);
};
#endif // MAINWINDOW_H
