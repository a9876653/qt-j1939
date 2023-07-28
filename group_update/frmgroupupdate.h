#ifndef FRMGROUPUPDATE_H
#define FRMGROUPUPDATE_H

#include <QWidget>
#include <QTimer>
#include <QCheckBox>
#include "mcheckbox.h"
#include "file_transfer_master_port.h"
#include "gourp_update.h"
namespace Ui
{
class FrmGroupUpdate;
}

typedef enum
{
    FILE_STEP_NONE = 0,
    FILE_STEP_HANDSHAKE,
    FILE_STEP_SEND_CMD,
    FILE_STEP_GET_FILE_INFO,
    FILE_STEP_GET_FILE_DATA,
} FILE_UPDATA_STEP;

class FrmGroupUpdate : public QWidget
{
    Q_OBJECT

public:
    explicit FrmGroupUpdate(QWidget *parent = nullptr);
    ~FrmGroupUpdate();

    void getCommData(uint msg_id, char *buff, uint16_t len);

private slots:
    void on_openPushButton_clicked();
    void slotUpdataProgress(float);
    void slotUpdataInfo(QString);
    void slotAppInfoUpdata(app_info_str_t &str);
    void slotUpdateState(uint8_t src, int result, uint32_t offset);
    void slotUpdateFailed(uint8_t src);
    void slotGroupUpdateState(update_info_t info);
    void slotTimerOut();
    void on_startPushButton_clicked();

    void on_clearPushButton_clicked();

    void on_getAppInfoPushButton_clicked();

    void on_stopPushButton_clicked();

    void on_handShakePushButton_clicked();

    void on_selectAllPushButton_clicked();

    void on_clrSelectPushButton_clicked();

    void on_startSpinBox_valueChanged(int arg1);

    void on_cntSpinBox_valueChanged(int arg1);

    void on_resetPushButton_clicked();

    void on_startUpdateBtn_clicked();

    void on_reqUpInfoBtn_clicked();

private:
    Ui::FrmGroupUpdate *ui;
    QString             file_path;
    QTimer *            timer        = nullptr;
    uint32_t            sendCmdCount = 0;
    FILE_UPDATA_STEP    updataStep   = FILE_STEP_NONE;
    QList<MCheckBox *>  update_obj_list;
    file_data_info_t *  file_data_info;
    FileTransferMaster  filetransfer;
    GroupUpdate         group_date;

private:
    void             disablePushButton();
    void             enablePushButton();
    void             create_update_obj();
    QVector<uint8_t> get_update_objs();
    MCheckBox *      obj_addr_find(uint8_t obj_addr);
};

#endif // FRMGROUPUPDATE_H
