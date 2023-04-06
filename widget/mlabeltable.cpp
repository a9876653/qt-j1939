#include "mlabeltable.h"
#include "ui_mlabeltable.h"

MLabelTable::MLabelTable(QString des, int column_count, QWidget *parent)
    : ui(new Ui::MLabelTable), column_count(column_count), QWidget(parent)
{
    ui->setupUi(this);
    ui->groupBox->setTitle(des);
}

MLabelTable::~MLabelTable()
{
    delete ui;
}

void MLabelTable::insert(MLabel *label)
{
    int row    = widget_count / column_count;
    int column = widget_count % column_count;
    ui->gridLayout_4->addWidget(label, row, column);
    widget_count++;
}

int MLabelTable::get_row_cnt()
{
    return widget_count / column_count;
}
