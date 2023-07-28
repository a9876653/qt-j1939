#include "frmgroupupdate.h"
#include "ui_frmgroupupdate.h"
#include "QFileDialog"

#define TEMMP_PATH "./cfg/tempFileTransferPath.ini"

FrmGroupUpdate::FrmGroupUpdate(QWidget *parent) : QWidget(parent), ui(new Ui::FrmGroupUpdate)
{
    ui->setupUi(this);

    QFile tempFile(TEMMP_PATH);
    if (tempFile.open(QIODevice::ReadWrite))
    {
        file_path = tempFile.read(1024);
    }

    ui->pathTextEdit->setText(file_path);
    file_data_info = set_file_data_info(file_path);
    slotAppInfoUpdata(file_data_info->app_info);

    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &FrmGroupUpdate::slotTimerOut);
    connect(&group_date, &GroupUpdate::sig_dbg_info, this, &FrmGroupUpdate::slotUpdataInfo);
    connect(&filetransfer, &FileTransferMaster::sig_dbg_info, this, &FrmGroupUpdate::slotUpdataInfo);
    connect(&filetransfer, &FileTransferMaster::sig_app_info_update, this, &FrmGroupUpdate::slotAppInfoUpdata);
    connect(&filetransfer, &FileTransferMaster::sig_update_failed, this, &FrmGroupUpdate::slotUpdateFailed);
    connect(&filetransfer, &FileTransferMaster::sig_update_state, this, &FrmGroupUpdate::slotUpdateState);
    connect(&group_date, &GroupUpdate::sig_update_state, this, &FrmGroupUpdate::slotGroupUpdateState);
    create_update_obj();
}

FrmGroupUpdate::~FrmGroupUpdate()
{
    delete ui;
}

void FrmGroupUpdate::on_openPushButton_clicked()
{
    QString define_path = "*.*";
    QFile   tempFile(TEMMP_PATH);
    if (tempFile.open(QIODevice::ReadWrite))
    {
        define_path = tempFile.read(1024);
    }
    file_path = QFileDialog::getOpenFileName(this, "请选择文件", define_path, "*.hx*");
    if (file_path.isEmpty())
    {
        tempFile.close();
        return;
    }
    if (tempFile.isOpen())
    {
        tempFile.resize(0);
        tempFile.write(file_path.toLatin1());
        tempFile.close();
    }
    ui->pathTextEdit->setText(file_path);
    file_data_info = set_file_data_info(file_path);
    slotAppInfoUpdata(file_data_info->app_info);
}

void FrmGroupUpdate::slotTimerOut()
{
    filetransfer.transfer_task();
    slotUpdataProgress(0);
}

void FrmGroupUpdate::disablePushButton()
{
    ui->openPushButton->setEnabled(false);
    ui->startPushButton->setEnabled(false);
}

void FrmGroupUpdate::enablePushButton()
{
    ui->openPushButton->setEnabled(true);
    ui->startPushButton->setEnabled(true);
}

QVector<uint8_t> FrmGroupUpdate::get_update_objs()
{
    QVector<uint8_t> obj_addr;
    for (int i = 0; i < update_obj_list.count(); i++)
    {
        MCheckBox *obj = update_obj_list.at(i);
        if (obj->isChecked())
        {
            uint8_t v = obj->text().toInt();
            obj->set_icon(QPixmap(":/icons/true_icon"), "更新中");
            obj_addr.append(v);
        }
        else
        {
            obj->set_icon(QPixmap(""), "");
        }
    }
    return obj_addr;
}

void FrmGroupUpdate::on_startPushButton_clicked()
{
    QVector<uint8_t> obj_addr = get_update_objs();
    if (obj_addr.count() > 0)
    {
        disablePushButton();
        file_data_info = filetransfer.set_file_data_info(file_path);

        filetransfer.start_transfer(obj_addr.data(), obj_addr.count(), file_data_info);
        slotAppInfoUpdata(file_data_info->app_info);
        timer->start(5);
    }
    else
    {
        slotUpdataInfo(QString("请选择需升级的节点"));
    }
}

void FrmGroupUpdate::on_resetPushButton_clicked()
{
    filetransfer.verify_request();
}

void FrmGroupUpdate::slotUpdataProgress(float value)
{
    (void)value;
    value = filetransfer.get_update_process();
    ui->updateProgressBar->setValue(value);
    if (value >= 100)
    {
        enablePushButton();
    }
}

void FrmGroupUpdate::on_clearPushButton_clicked()
{
    ui->debugTextEdit->clear();
}

void FrmGroupUpdate::slotUpdataInfo(QString str)
{
    ui->debugTextEdit->append(str);
}

void FrmGroupUpdate::on_getAppInfoPushButton_clicked()
{
    filetransfer.app_info_request();
}

void FrmGroupUpdate::slotAppInfoUpdata(app_info_str_t &str)
{
    ui->appNameTextEdit->setText(str.appName);
    ui->boardIdTextEdit->setText(str.boardId);
    ui->startAddrTextEdit->setText(str.startAddr);
    ui->sizeTextEdit->setText(str.size);
    ui->boardVerTextEdit->setText(str.boardVer);
    ui->firmwareVerTextEdit->setText(str.firmwareVer);
    ui->buildTimeTextEdit->setText(str.buildTime);
    ui->gitCommitTextEdit->setText(str.gitCommit);

    //    slotUpdataInfo("App Name: " + str.appName);
    //    slotUpdataInfo("App Board Id: " + str.boardId);
    //    slotUpdataInfo("App Start Address: " + str.startAddr);
    //    slotUpdataInfo("App Size: " + str.size);
    //    slotUpdataInfo("App Crc: " + str.crc);
    //    slotUpdataInfo("Board Ver: " + str.boardVer);
    //    slotUpdataInfo("Firmware Ver: " + str.firmwareVer);
    //    slotUpdataInfo("App Build Time:" + str.buildTime);
    //    slotUpdataInfo("App Git Commit:" + str.gitCommit);
}

void FrmGroupUpdate::slotUpdateState(uint8_t src, int result, uint32_t offset)
{
    (void)src;
    (void)result;
    (void)offset;
    if (result == FILE_TRANSFER_VERIFY)
    {
        for (int i = 0; i < update_obj_list.count(); i++)
        {
            MCheckBox *obj = update_obj_list.at(i);

            if (src == obj->text().toInt())
            {
                obj->set_icon(QPixmap(":/icons/true_icon"), "完成");
                break;
            }
        }
    }
}

void FrmGroupUpdate::slotUpdateFailed(uint8_t src)
{
    if (src == 0xFF)
    {
        on_stopPushButton_clicked();
        for (int i = 0; i < update_obj_list.count(); i++)
        {
            MCheckBox *obj = update_obj_list.at(i);

            if (obj && obj->isChecked())
            {
                obj->set_icon(QPixmap(":/icons/false_icon"), "更新失败");
            }
        }
        slotUpdataInfo(QString("update failed"));
    }
    else
    {
        for (int i = 0; i < update_obj_list.count(); i++)
        {
            MCheckBox *obj = update_obj_list.at(i);

            if (src == obj->text().toInt())
            {
                obj->set_icon(QPixmap(":/icons/false_icon"), "更新失败");
                break;
            }
        }
        slotUpdataInfo(QString("src %1 update failed").arg(src));
    }
}

void FrmGroupUpdate::on_stopPushButton_clicked()
{
    enablePushButton();
    timer->stop();
}

void FrmGroupUpdate::on_handShakePushButton_clicked()
{
    QVector<uint8_t> obj_addr = get_update_objs();
    filetransfer.send_handshake_cmd(obj_addr.data(), obj_addr.count());
}

void FrmGroupUpdate::create_update_obj()
{
    int update_obj_num = ui->cntSpinBox->value();
    int start_obj_num  = ui->startSpinBox->value();
    while (update_obj_list.count() > update_obj_num)
    {
        MCheckBox *obj = update_obj_list.last();
        ui->gridLayout_2->removeWidget(obj);
        update_obj_list.removeOne(obj);
        delete obj;
    }
    while (update_obj_list.count() < update_obj_num)
    {
        int cnt = update_obj_list.count();

        MCheckBox *obj = new MCheckBox();
        ui->gridLayout_2->addWidget(obj, (cnt / 5), (cnt % 5));
        update_obj_list.append(obj);
    }
    for (int i = 0; i < update_obj_num; i++)
    {
        MCheckBox *obj = update_obj_list.at(i);
        QString    s   = QString("%1").arg(i + start_obj_num);
        obj->setText(s);
    }
}

void FrmGroupUpdate::on_selectAllPushButton_clicked()
{
    for (int i = 0; i < update_obj_list.count(); i++)
    {
        MCheckBox *obj = update_obj_list.at(i);
        obj->setChecked(true);
    }
}

void FrmGroupUpdate::on_clrSelectPushButton_clicked()
{
    for (int i = 0; i < update_obj_list.count(); i++)
    {
        MCheckBox *obj = update_obj_list.at(i);
        obj->setChecked(false);
    }
}

void FrmGroupUpdate::on_startSpinBox_valueChanged(int arg1)
{
    (void)arg1;
    create_update_obj();
}

void FrmGroupUpdate::on_cntSpinBox_valueChanged(int arg1)
{
    (void)arg1;
    create_update_obj();
}

MCheckBox *FrmGroupUpdate::obj_addr_find(uint8_t obj_addr)
{
    for (int i = 0; i < update_obj_list.count(); i++)
    {
        MCheckBox *obj = update_obj_list.at(i);
        if (obj && obj->text().toInt() == obj_addr)
        {
            return obj;
        }
    }
    return NULL;
}

void FrmGroupUpdate::slotGroupUpdateState(update_info_t info)
{
    ui->groupUpdateProgressBar->setValue(info.process);
    if (info.process >= 100)
    {
        for (int i = 0; i < info.obj_num; i++)
        {
            MCheckBox *obj = obj_addr_find(info.obj_addr[i]);
            if (obj && obj->isChecked())
            {
                obj->set_icon(QPixmap(":/icons/true_icon"), "从机更新完成");
            }
        }
    }
    for (int i = 0; i < info.obj_num; i++)
    {
        MCheckBox *obj = obj_addr_find(info.obj_addr[i] & 0x7F);
        if (obj && obj->isChecked())
        {
            obj->set_icon(QPixmap(":/icons/true_icon"), "从机更新完成");
        }
    }
    for (int i = 0; i < update_obj_list.count(); i++)
    {
        MCheckBox *obj = update_obj_list.at(i);
        if (obj && obj->isChecked())
        {
            int j = 0;
            // 选定的节点是否在读取的列表中
            for (j = 0; j < info.obj_num; j++)
            {
                uint addr = obj->text().toUInt();
                if ((info.obj_addr[j] & 0x7F) == addr)
                {
                    if (info.obj_addr[j] & 0x80)
                    {
                        obj->set_icon(QPixmap(":/icons/false_icon"), "从机文件校验失败");
                    }
                    break;
                }
            }

            if (j >= info.obj_num)
            {
                obj->set_icon(QPixmap(":/icons/false_icon"), "从机更新失败");
            }
        }
    }
}

void FrmGroupUpdate::on_startUpdateBtn_clicked()
{
    group_date.start(get_update_objs());
}

void FrmGroupUpdate::on_reqUpInfoBtn_clicked()
{
    group_date.request_update_info();
}
