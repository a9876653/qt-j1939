#include "pagemsgdisplay.h"
#include "ui_pagemsgdisplay.h"

PageMsgDisplay::PageMsgDisplay(QMap<uint, MsgData *> msgs_map) : msgs_map(msgs_map), ui(new Ui::PageMsgDisplay)
{
    ui->setupUi(this);

    int cloumnIndex = 0;
    int rowIndex    = 0;
    for (MsgData *msg_data : msgs_map)
    {
        cloumnIndex               = 0;
        QTreeWidgetItem *treeItem = new QTreeWidgetItem;
        ui->treeWidget->addTopLevelItem(treeItem);
        treeItem->setData(0, Qt::UserRole + 1, QVariant::fromValue(msg_data));
        QTreeWidgetItem *treeChildItem = nullptr;

        treeItem->setText(cloumnIndex++, msg_data->name);
        treeItem->setText(cloumnIndex++, QString("0x%1").arg(msg_data->id, 8, 16, QLatin1Char('0')));
        treeItem->setText(cloumnIndex++, QString("0x%1").arg(msg_data->pgn, 4, 16, QLatin1Char('0')));

        uint16_t list_count = msg_data->signals_list.count();
        for (uint16_t i = 0; i < list_count; i++)
        {
            Signal *msg_sig = msg_data->signals_list.at(i);
            // MsgData::Signal *msg_sig  = &_msg_sig;
            treeChildItem = new QTreeWidgetItem;
            treeItem->addChild(treeChildItem);

            treeChildItem->setText(cloumnIndex++, msg_sig->name);

            ui->treeWidget->setItemWidget(treeChildItem, cloumnIndex++, msg_sig->rec_hex_widget.text);
            ui->treeWidget->setItemWidget(treeChildItem, cloumnIndex++, msg_sig->rec_dec_widget.text);
            ui->treeWidget->setItemWidget(treeChildItem, cloumnIndex++, msg_sig->send_widget.text);
            rowIndex++;
            cloumnIndex = 3;
        }
    }
    ui->treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

PageMsgDisplay::~PageMsgDisplay()
{
    delete ui;
}

void PageMsgDisplay::on_expandPushButton_clicked()
{
    ui->treeWidget->expandAll();
}

void PageMsgDisplay::on_collapsPpushButton_clicked()
{
    ui->treeWidget->collapseAll();
}

void PageMsgDisplay::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    (void)column;
    MsgData *data = item->data(0, Qt::UserRole + 1).value<MsgData *>();
    if (data == nullptr && item->parent() != nullptr) // 找不到用户数据,往父对象再找一遍
    {
        item = item->parent();
        data = item->data(0, Qt::UserRole + 1).value<MsgData *>();
    }
    if (data != nullptr)
    {
        data->slot_request_pgn();
    }
}
