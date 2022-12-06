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

private:
    Ui::MainWindow *ui;

    QTimer test_timer;

    bool device_status = false;

    MsgSignals msgs;

    void slot_test_send();
};
#endif // MAINWINDOW_H
