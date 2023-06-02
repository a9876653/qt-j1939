#include "frmcentericon.h"
#include "ui_frmcentericon.h"
#include "QDebug"

frmCenterIcon::frmCenterIcon(QPixmap icon, QWidget *parent) : QWidget(parent), ui(new Ui::frmCenterIcon)
{
    ui->setupUi(this);
    set_icon(icon);
}

frmCenterIcon::~frmCenterIcon()
{
    delete ui;
}

void frmCenterIcon::set_icon(QPixmap icon, QString str)
{
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
    ui->iconLabel->setPixmap(icon);
    ui->textLabel->setText(str);
}

void frmCenterIcon::resize_icon()
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
