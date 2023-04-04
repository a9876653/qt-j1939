#include "mvaluelabel.h"
#include "ui_mvaluelabel.h"

MValueLabel::MValueLabel(bool read_only, QWidget *parent) : QWidget(parent), ui(new Ui::MValueLabel)
{
    ui->setupUi(this);
    ui->valueLabel->setText("#");
}

MValueLabel::~MValueLabel()
{
    delete ui;
}

void MValueLabel::slot_update(QVariant v)
{
    value = v;
    ui->valueLabel->setText(value.toString());
    emit sig_update(v);
}

void MValueLabel::set_backcolor(QColor color)
{
    QString color_str = QString("QLabel{background-color:rgb(%1,%2,%3);}").arg(color.red()).arg(color.green()).arg(color.blue());
    ui->valueLabel->setStyleSheet(color_str); //设置样式表
}
