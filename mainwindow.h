#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "msg_signals.h"
#include "pagemsgdisplay.h"

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

private:
    Ui::MainWindow *ui;

    QTimer test_timer;

    bool device_status = false;

    MsgSignals msgs;

    QMap<uint, PageMsgDisplay *> src_page_map;

    void slot_test_send();
};
#endif // MAINWINDOW_H
