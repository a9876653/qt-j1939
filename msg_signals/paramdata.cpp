#include "paramdata.h"
#include "canard_dsdl.h"
#include "QDebug"

#define PAGEDATA_DBG(x...) // qDebug(x)

ParamData::ParamData(int id, QString name, QString type, float def_v) : id(id), name(name), type(type)
{
    connect(&send_widget, &CustomTextWidget::sig_enter_event, this, &ParamData::slot_request_write);
    icon_widget = new frmCenterIcon(QPixmap(""));
    recv_widget.text->setFixedWidth(160);
    send_widget.text->setFixedWidth(160);
    send_widget.text->setText(QString("%1").arg(def_v, 12, 'f', 0));
    if (type.contains("32"))
    {
        reg_len = 2;
    }
    else
    {
        reg_len = 1;
    }
}

void ParamData::slot_set_icon(bool ok)
{
    if (ok)
    {
        icon_widget->set_icon(QPixmap(":/icons/true_icon"));
    }
    else
    {
        icon_widget->set_icon(QPixmap(":/icons/false_icon"));
    }
}

void ParamData::slot_request_write()
{
    icon_widget->set_icon(QPixmap(":/icons/false_icon"));
    QString dec_s = send_widget.text->toPlainText();
    write_value   = dec_s.toInt();

    emit sig_request_write_reg(id, (uint16_t *)&write_value, reg_len);

    PAGEDATA_DBG("%s", QString("请求设置参数 %1 : id %2, value %3").arg(name).arg(id).arg(write_value).toStdString().c_str());
}

void ParamData::slot_request_read()
{
    icon_widget->set_icon(QPixmap(":/icons/false_icon"));

    emit sig_request_read_reg(id, reg_len);

    PAGEDATA_DBG("%s", QString("请求获取参数 %1 : id %2, value %3").arg(name).arg(id).toStdString().c_str());
}

void ParamData::slot_write_finish()
{
    slot_request_read();
    icon_widget->set_icon(QPixmap(":/icons/true_icon"));
    PAGEDATA_DBG("%s", QString("写入参数 %1 : id %2, value %3").arg(name).arg(id).arg(write_value).toStdString().c_str());
}
void ParamData::slot_read_finish(uint32_t value)
{
    recv_widget.text->setText(QString(" %1 (0x%2)").arg(value).arg(value, reg_len * 4, 16, QLatin1Char('0')));
    icon_widget->set_icon(QPixmap(":/icons/true_icon"));
    PAGEDATA_DBG("%s", (QString("获取参数 %1 : id %2, value %3").arg(name).arg(id).arg(write_value)).toStdString().c_str());
}
