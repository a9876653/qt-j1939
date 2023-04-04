#include "mtableview.h"
#include "ui_mtableview.h"

MTableView::MTableView(QString des, int row_cnt, int cloumn_cnt, QString row_header, QString cloumn_header, QWidget *parent)
    : QWidget(parent), ui(new Ui::MTableView), row_cnt(row_cnt), cloumn_cnt(cloumn_cnt + 1)
{
    ui->setupUi(this);
    ui->groupBox->setTitle(des);
    normal_brush = QBrush(Qt::white);
    max_brush    = QBrush(QColor(255, 0, 0));
    min_brush    = QBrush(QColor(185, 255, 170));
    data_model   = new QStandardItemModel();
    data_model->setHorizontalHeaderItem(0, new QStandardItem(""));
    for (int i = 1; i <= cloumn_cnt; i++)
    {
        QString s = QString("%1 %2").arg(cloumn_header).arg(i);
        data_model->setHorizontalHeaderItem(i, new QStandardItem(s));
    }
    for (int i = 0; i < row_cnt; i++)
    {
        QString s = QString("%1 %2").arg(row_header).arg(i + 1);
        data_model->setItem(i, 0, new QStandardItem(s));
        for (int j = 1; j <= cloumn_cnt; j++)
        {
            QStandardItem *item = new QStandardItem("0");
            data_model->setItem(i, j, item);
            item->setTextAlignment(Qt::AlignCenter);
        }
    }
    data_model->setRowCount(row_cnt);
    ui->tableView->setModel(data_model);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers); // 只读
    // ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 自动列宽
    ui->tableView->resizeColumnsToContents();
}

MTableView::~MTableView()
{
    delete ui;
}
int MTableView::index2row(int index)
{
    return index / (cloumn_cnt - 1);
}
int MTableView::index2cloumn(int index)
{
    return (index % (cloumn_cnt - 1)) + 1;
}
void MTableView::set_item_backcolor(int index, QBrush brush)
{
    data_model->item(index2row(index), index2cloumn(index))->setBackground(brush);
}
void MTableView::set_item_value(int index, QVariant value)
{
    data_model->item(index2row(index), index2cloumn(index))->setText(value.toString());
}

void MTableView::slot_update(int index, QVariant value)
{
    if (value > max)
    {
        max = value;
        set_item_backcolor(max_index, normal_brush);
        max_index = index;
        set_item_backcolor(max_index, max_brush);
    }
    else if (value < min)
    {
        min = value;
        set_item_backcolor(min_index, normal_brush);
        min_index = index;
        set_item_backcolor(min_index, min_brush);
    }
    set_item_value(index, value);
}
