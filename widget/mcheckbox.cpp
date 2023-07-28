#include "mcheckbox.h"
#include "ui_mcheckbox.h"

MCheckBox::MCheckBox(QPixmap icon, QWidget *parent) : QWidget(parent), ui(new Ui::MCheckBox)
{
    ui->setupUi(this);
    set_icon(icon);
}

MCheckBox::~MCheckBox()
{
    delete ui;
}

void MCheckBox::setChecked(bool state)
{
    ui->checkBox->setChecked(state);
}

QString MCheckBox::text()
{
    return ui->checkBox->text();
}

void MCheckBox::setText(QString &text)
{
    ui->checkBox->setText(text);
}

bool MCheckBox::isChecked()
{
    return ui->checkBox->isChecked();
}

void MCheckBox::set_icon(QPixmap icon, QString str)
{
    ui->textLabel->setText(str);
    if (icon.isNull())
    {
        return;
    }
    pixmap = icon;

    int    height = ui->iconLabel->size().height() - 8;
    int    width  = ui->iconLabel->size().width() - 8;
    double k      = 1;
    /** 以较小值作为缩放系数 **/
    if (icon.height() > 0 && icon.width() > 0)
    {
        if (height < width)
        {
            k = double(height) / icon.height();
        }
        else
        {
            k = double(width) / icon.width();
        }
        icon = icon.scaled(icon.height() * k, icon.width() * k);
    }
    else
    {
        qDebug("icon size is zero!");
    }
    ui->iconLabel->setPixmap(icon);
}

void MCheckBox::resize_icon()
{
    QPixmap icon   = pixmap;
    int     height = ui->iconLabel->size().height();
    int     width  = ui->iconLabel->size().width();
    double  k      = 1;
    /** 以较小值作为缩放系数 **/
    if (height < width)
    {
        k = double(height) / icon.height();
    }
    else
    {
        k = double(width) / icon.width();
    }
    icon = icon.scaled(icon.height() * k, icon.width() * k);
    ui->iconLabel->setPixmap(icon);
}
