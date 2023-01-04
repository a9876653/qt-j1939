#include "paramdata.h"
#include "canard_dsdl.h"
#include "QDebug"

#define PAGEDATA_DBG(x...) qDebug(x)

ParamData::ParamData(int id, QString name, float def_v) : id(id), name(name)
{
    send_widget.text->setText(QString("%1").arg(def_v));
    connect(&send_widget, &CustomTextWidget::sig_enter_event, this, &ParamData::slot_encode_send);
}

void ParamData::slot_encode_send()
{
    QString dec_s = send_widget.text->toPlainText();
    int     value = dec_s.toInt();
    canardDSDLSetIxx(encode_buff, 0, id, 32);
    canardDSDLSetIxx(encode_buff, 32, value, 32);
    emit sig_msg_send(0xF003, encode_buff, 8);
    PAGEDATA_DBG("请求设置参数 %s :id %d, value %d", name.toLatin1().data(), id, value);
}

void ParamData::request_get_value()
{
    canardDSDLSetIxx(encode_buff, 0, id, 32);
    canardDSDLSetIxx(encode_buff, 32, 0, 32);
    emit sig_msg_send(0xF001, encode_buff, 8);
    PAGEDATA_DBG("请求获取参数 %s :id %d", name.toLatin1().data(), id);
}

void ParamData::set_recv_value(int value)
{
    recv_widget.text->setText(QString("%1").arg(value));
    PAGEDATA_DBG("获取参数 %s :id %d, value %d", name.toLatin1().data(), id, value);
}
