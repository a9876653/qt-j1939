#include "enumvalueveiw.h"
#include "ui_enumvalueveiw.h"

EnumValueVeiw::EnumValueVeiw(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EnumValueVeiw)
{
    ui->setupUi(this);
}

EnumValueVeiw::~EnumValueVeiw()
{
    delete ui;
}
