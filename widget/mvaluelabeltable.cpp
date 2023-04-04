#include "mvaluelabeltable.h"
#include "ui_mvaluelabeltable.h"

MValueLabelTable::MValueLabelTable(QString  des,
                                   int      row_cnt,
                                   int      cloumn_cnt,
                                   QString  row_header,
                                   QString  cloumn_header,
                                   QWidget *parent)
    : QWidget(parent), ui(new Ui::MValueLabelTable), row_cnt(row_cnt), cloumn_cnt(cloumn_cnt + 1)
{
    ui->setupUi(this);
    ui->groupBox->setTitle(des);
    normal_brush = QBrush(Qt::white);
    max_brush    = QBrush(QColor(255, 0, 0));
    min_brush    = QBrush(QColor(185, 255, 170));
    ui->tableWidget->setColumnCount(cloumn_cnt + 1);
    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
    for (int i = 1; i <= cloumn_cnt; i++)
    {
        QString s = QString("%1 %2").arg(cloumn_header).arg(i);
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(s));
    }
    ui->tableWidget->setRowCount(row_cnt);
    for (int i = 0; i < row_cnt; i++)
    {
        QString s = QString("%1 %2").arg(row_header).arg(i + 1);
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(s));
    }
    ui->tableWidget->resizeColumnsToContents();
}

MValueLabelTable::~MValueLabelTable()
{
    delete ui;
}

void MValueLabelTable::insert(int row, int column, MValueLabel *label)
{
    ui->tableWidget->setCellWidget(row, column + 1, label);

    connect(label, &MValueLabel::sig_update, this, &MValueLabelTable::slot_update);
}

void MValueLabelTable::slot_update(QVariant value)
{
    MValueLabel *item = qobject_cast<MValueLabel *>(sender());
    if (value > max)
    {
        if (max_item == nullptr)
        {
            max_item = item;
        }
        max = value;
        max_item->set_backcolor(normal_brush.color());
        max_item = item;
        max_item->set_backcolor(max_brush.color());
    }
    else if (value < min)
    {
        if (min_item == nullptr)
        {
            min_item = item;
        }
        min = value;
        min_item->set_backcolor(normal_brush.color());
        min_item = item;
        min_item->set_backcolor(min_brush.color());
    }
}
