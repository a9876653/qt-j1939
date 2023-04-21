#include "mlabel.h"
#include "ui_mlabel.h"

MLabel::MLabel(QString des, ValueDes value_des, QWidget *parent) : QWidget(parent), ui(new Ui::MLabel), value_des(value_des)
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
    QString s;
    uint    v = value.toUInt();
    s += QString("%1(0x%2)").arg(value.toString()).arg(v, 4, 16, QLatin1Char('0'));
    if (value_des.is_bit_des)
    {
        for (int i = 0; i < 32; i++)
        {
            QVariant bit_v = QVariant(i);
            if ((v & (1 << i)) && value_des.value_des_map.contains(QVariant(bit_v)))
            {
                s += QString("\nBit%1:%2").arg(i).arg(value_des.value_des_map.value(bit_v));
            }
        }
    }
    else
    {
        if (value_des.value_des_map.contains(value))
        {
            s += QString("--%1").arg(value_des.value_des_map.value(value));
        }
    }
    ui->valueLabel->setText(s);
}
