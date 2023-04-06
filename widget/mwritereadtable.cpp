#include "mwritereadtable.h"
#include "ui_mwritereadtable.h"

MWriteReadTable::MWriteReadTable(QString node, QWidget *parent) : QWidget(parent), ui(new Ui::MWriteReadTable)
{
    ui->setupUi(this);
    ui->groupBox->setTitle(node);
    connect(&read_timer, &QTimer::timeout, this, &MWriteReadTable::slot_read_timeout);
    connect(&write_timer, &QTimer::timeout, this, &MWriteReadTable::slot_write_timeout);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //先自适应宽度
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents); //然后设置要根据内容使用宽度的列
}

MWriteReadTable::~MWriteReadTable()
{
    delete ui;
}

void MWriteReadTable::showEvent(QShowEvent *event)
{
    (void)event;
    // on_readAllBtn_clicked();
}
void MWriteReadTable::hideEvent(QHideEvent *event)
{
    (void)event;
    read_timer.stop();
}

void MWriteReadTable::insert(MWriteReadWidget *ins)
{
    int row    = ui->tableWidget->rowCount();
    int cloumn = 0;
    ui->tableWidget->setRowCount(row + 1);
    ui->tableWidget->setCellWidget(row, cloumn++, ins->des_label);
    ui->tableWidget->setCellWidget(row, cloumn++, ins->read_spinbox);
    ui->tableWidget->setCellWidget(row, cloumn++, ins->write_spinbox);
    ui->tableWidget->setCellWidget(row, cloumn++, ins->read_btn);
    ui->tableWidget->setCellWidget(row, cloumn++, ins->write_btn);
    ui->tableWidget->setCellWidget(row, cloumn++, ins->text_icon);
    ins_list.append(ins);
}

void MWriteReadTable::slot_read_timeout()
{
    if (read_index < ins_list.count())
    {
        MWriteReadWidget *ins = ins_list.at(read_index);

        emit ins->read_btn->clicked();
        read_index++;
    }
    else
    {
        read_timer.stop();
    }
}
void MWriteReadTable::slot_write_timeout()
{
    if (write_index < ins_list.count())
    {
        MWriteReadWidget *ins = ins_list.at(write_index);

        emit ins->write_btn->clicked();
        write_index++;
    }
    else
    {
        write_timer.stop();
    }
}

void MWriteReadTable::on_readAllBtn_clicked()
{
    read_index = 0;
    read_timer.start(50);
}

void MWriteReadTable::on_writeAllBtn_clicked()
{
    write_index = 0;
    write_timer.start(50);
}

int MWriteReadTable::get_row_cnt()
{
    return ui->tableWidget->rowCount();
}
