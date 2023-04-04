#include "mlabel.h"
#include "ui_mlabel.h"

MLabel::MLabel(QString des, QWidget *parent) : QWidget(parent), ui(new Ui::MLabel)
{
    ui->setupUi(this);
    ui->desLabel->setText(des);
    ui->valueLabel->setText("0");
}

MLabel::~MLabel()
{
    delete ui;
}

void MLabel::slot_update(QVariant value)
{
    QString s = QString("%1(0x%2)").arg(value.toString()).arg(value.toUInt(), 4, 16, QLatin1Char('0'));
    ui->valueLabel->setText(s);
}
