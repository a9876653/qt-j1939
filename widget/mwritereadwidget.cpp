#include "mwritereadwidget.h"

MWriteReadWidget::MWriteReadWidget(QString des, QVariant min, QVariant max, QVariant def, QVariant write_value)
    : des(des), min(min), max(max), def(def)
{
    text_icon     = new frmCenterIcon(QPixmap(":/icons/true_icon"));
    des_label     = new QLabel(des);
    write_spinbox = new MQSpinBox();
    read_spinbox  = new MQSpinBox();
    write_btn     = new QToolButton();
    read_btn      = new QToolButton();

    write_btn->setText("写入");
    read_btn->setText("读取");

    write_spinbox->setMaximum(max.toInt());
    write_spinbox->setMinimum(min.toInt());
    write_spinbox->setValue(write_value.toInt());

    read_spinbox->setReadOnly(true);                                            // 只读
    read_spinbox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons); // 无按钮
    read_spinbox->setMaximum(max.toInt());
    read_spinbox->setMinimum(min.toInt());
    read_spinbox->setValue(def.toInt());

    text_icon->set_icon(QPixmap(":/icons/true_icon"));

    connect(read_btn, &QToolButton::clicked, this, &MWriteReadWidget::on_readBtn_clicked);
    connect(write_btn, &QToolButton::clicked, this, &MWriteReadWidget::update);
    // connect(write_spinbox, &QSpinBox::editingFinished, this, &MWriteReadWidget::update);
}

MWriteReadWidget::~MWriteReadWidget()
{
}

void MWriteReadWidget::update()
{
    text_icon->set_icon(QPixmap(":/icons/false_icon"), "正在写入");
    emit sig_update(write_spinbox->value());
}

void MWriteReadWidget::slot_update_finish()
{
    text_icon->set_icon(QPixmap(":/icons/true_icon"), "写入成功");
}

void MWriteReadWidget::slot_update(QVariant value)
{
    read_spinbox->setValue(value.toInt());
    text_icon->set_icon(QPixmap(":/icons/true_icon"), "读取成功");
}

void MWriteReadWidget::on_readBtn_clicked()
{
    text_icon->set_icon(QPixmap(":/icons/false_icon"), "正在读取");
    emit sig_request_read();
}
