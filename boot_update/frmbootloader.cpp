#include "frmbootloader.h"
#include "QFileDialog"
#include "ui_frmbootloader.h"
#include "middle_signal.h"

frmBootloader::frmBootloader(QWidget *parent) : QWidget(parent), ui(new Ui::frmBootloader)
{
    ui->setupUi(this);

    QFile tempFile("./cfg/tempFilePath.ini");
    if (tempFile.open(QIODevice::ReadWrite))
    {
        file_path = tempFile.read(1024);
    }
    boot_port_init();

    ui->pathTextEdit->setText(file_path);
    file_data_info = set_file_data_info(file_path);
    slotAppInfoUpdata(file_data_info->app_info);

    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &frmBootloader::slotTimerOut);
    connect(&cycle_timer, &QTimer::timeout, this, &frmBootloader::slotHandshakeTimeout);

    connect(MiddleSignalIns, &MiddleSignal::sig_dbg_info, this, &frmBootloader::slotUpdataInfo);
    connect(MiddleSignalIns, &MiddleSignal::sig_app_info_update, this, &frmBootloader::slotAppInfoUpdata);
    connect(MiddleSignalIns, &MiddleSignal::sig_update_failed, this, &frmBootloader::on_stopPushButton_clicked);
    create_update_obj();
}

frmBootloader::~frmBootloader()
{
    delete ui;
}

void frmBootloader::on_openPushButton_clicked()
{
    QString define_path = "*.*";
    QFile   tempFile("./cfg/tempFilePath.ini");
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

void frmBootloader::slotTimerOut()
{
    boot_port_poll();
    slotUpdataProgress(0);
}

void frmBootloader::disablePushButton()
{
    ui->openPushButton->setEnabled(false);
    ui->startPushButton->setEnabled(false);
    ui->jumpAppPushButton->setEnabled(false);
    ui->enterBootPushButton->setEnabled(false);
}

void frmBootloader::enablePushButton()
{
    ui->openPushButton->setEnabled(true);
    ui->startPushButton->setEnabled(true);
    ui->jumpAppPushButton->setEnabled(true);
    ui->enterBootPushButton->setEnabled(true);
}

QVector<uint8_t> frmBootloader::get_update_objs()
{
    QVector<uint8_t> obj_addr;
    for (int i = 0; i < update_obj_list.count(); i++)
    {
        QCheckBox *obj = update_obj_list.at(i);
        if (obj->isChecked())
        {
            uint8_t v = obj->text().toInt();
            obj_addr.append(v);
        }
    }
    return obj_addr;
}

void frmBootloader::on_startPushButton_clicked()
{
    QVector<uint8_t> obj_addr = get_update_objs();
    if (obj_addr.count() > 0)
    {
        disablePushButton();
        file_data_info = set_file_data_info(file_path);
        boot_port_start(obj_addr.data(), obj_addr.count(), file_data_info);
        slotAppInfoUpdata(file_data_info->app_info);
        timer->start(5);
    }
    else
    {
        slotUpdataInfo(QString("请选择需升级的节点"));
    }
}

void frmBootloader::on_resetPushButton_clicked()
{
    QVector<uint8_t> obj_addr = get_update_objs();
    if (obj_addr.count() > 0)
    {
        boot_port_reset(obj_addr.data(), obj_addr.count());
    }
    else
    {
        slotUpdataInfo(QString("请选择需重启的节点"));
    }
}

void frmBootloader::slotUpdataProgress(float value)
{
    (void)value;
    value = get_update_process();
    ui->updateProgressBar->setValue(value);
    if (value >= 100)
    {
        enablePushButton();
    }
}

void frmBootloader::on_clearPushButton_clicked()
{
    ui->debugTextEdit->clear();
}

void frmBootloader::slotUpdataInfo(QString str)
{
    ui->debugTextEdit->append(str);
}

void frmBootloader::on_getAppInfoPushButton_clicked()
{
    app_info_request();
}

void frmBootloader::slotAppInfoUpdata(app_info_str_t &str)
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

void frmBootloader::on_jumpAppPushButton_clicked()
{
    enter_app();
}

void frmBootloader::on_enterBootPushButton_clicked()
{
    QVector<uint8_t> obj_addr = get_update_objs();
    enter_boot(obj_addr.data(), obj_addr.count());
}

void frmBootloader::slotHandshakeTimeout()
{
    QVector<uint8_t> obj_addr = get_update_objs();
    enter_boot(obj_addr.data(), obj_addr.count());
}

void frmBootloader::on_stopPushButton_clicked()
{
    enablePushButton();
    timer->stop();
    cycle_timer.stop();
}

void frmBootloader::on_handShakePushButton_clicked()
{
    QVector<uint8_t> obj_addr = get_update_objs();
    enter_boot(obj_addr.data(), obj_addr.count());
    cycle_timer.start(50);
}

void frmBootloader::create_update_obj()
{
    int update_obj_num = ui->cntSpinBox->value();
    int start_obj_num  = ui->startSpinBox->value();
    while (update_obj_list.count() > update_obj_num)
    {
        QCheckBox *obj = update_obj_list.last();
        ui->gridLayout_2->removeWidget(obj);
        update_obj_list.removeOne(obj);
        delete obj;
    }
    while (update_obj_list.count() < update_obj_num)
    {
        int cnt = update_obj_list.count();

        QCheckBox *obj = new QCheckBox();
        ui->gridLayout_2->addWidget(obj, (cnt / 5), (cnt % 5));
        update_obj_list.append(obj);
    }
    for (int i = 0; i < update_obj_num; i++)
    {
        QCheckBox *obj = update_obj_list.at(i);
        obj->setText(QString("%1").arg(i + start_obj_num));
    }
}

void frmBootloader::on_selectAllPushButton_clicked()
{
    for (int i = 0; i < update_obj_list.count(); i++)
    {
        QCheckBox *obj = update_obj_list.at(i);
        obj->setChecked(true);
    }
}

void frmBootloader::on_clrSelectPushButton_clicked()
{
    for (int i = 0; i < update_obj_list.count(); i++)
    {
        QCheckBox *obj = update_obj_list.at(i);
        obj->setChecked(false);
    }
}

void frmBootloader::on_startSpinBox_valueChanged(int arg1)
{
    (void)arg1;
    create_update_obj();
}

void frmBootloader::on_cntSpinBox_valueChanged(int arg1)
{
    (void)arg1;
    create_update_obj();
}
