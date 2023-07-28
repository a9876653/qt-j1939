#ifndef FRMBOOTLOADER_H
#define FRMBOOTLOADER_H

#include <QWidget>
#include <QTimer>
#include "boot_port.h"
#include "boot_master.h"
#include "QCheckBox"

namespace Ui
{
class frmBootloader;
}

typedef enum
{
    STEP_NONE = 0,
    STEP_HANDSHAKE,
    STEP_SEND_CMD,
    STEP_GET_FILE_INFO,
    STEP_GET_FILE_DATA,
} UPDATA_STEP;

class frmBootloader : public QWidget
{
    Q_OBJECT

public:
    explicit frmBootloader(QWidget *parent = nullptr);
    ~frmBootloader();
    void getCommData(uint msg_id, char *buff, uint16_t len);

private slots:
    void on_openPushButton_clicked();
    void slotUpdataProgress(float);
    void slotUpdataInfo(QString);
    void slotAppInfoUpdata(app_info_str_t &str);
    void slotTimerOut();
    void slotHandshakeTimeout();
    void on_startPushButton_clicked();

    void on_clearPushButton_clicked();

    void on_getAppInfoPushButton_clicked();

    void on_jumpAppPushButton_clicked();

    void on_enterBootPushButton_clicked();

    void on_stopPushButton_clicked();

    void on_handShakePushButton_clicked();

    void on_selectAllPushButton_clicked();

    void on_clrSelectPushButton_clicked();

    void on_startSpinBox_valueChanged(int arg1);

    void on_cntSpinBox_valueChanged(int arg1);

    void on_resetPushButton_clicked();

private:
    Ui::frmBootloader *ui;
    QString            file_path;
    QTimer *           timer = nullptr;
    QTimer             cycle_timer;
    uint32_t           sendCmdCount = 0;
    UPDATA_STEP        updataStep   = STEP_NONE;
    QList<QCheckBox *> update_obj_list;
    file_data_info_t * file_data_info;
    void               disablePushButton();
    void               enablePushButton();
    void               create_update_obj();
    QVector<uint8_t>   get_update_objs();
};

#endif // FRMBOOTLOADER_H
