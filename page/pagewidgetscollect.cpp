#include "pagewidgetscollect.h"
#include "ui_pagewidgetscollect.h"
#include "filehandle.h"
#include "canard_dsdl.h"
#include "j1939_modbus_master.h"
#include "QDebug"
#include "mlabel.h"
#include "mlabeltable.h"
#include "mtableview.h"
#include "mvaluelabeltable.h"
#include "mwritereadtable.h"
#include <QLayout>

PageWidgetsCollect::PageWidgetsCollect(DataObjMap *data, QWidget *parent)
    : QWidget(parent), ui(new Ui::PageWidgetsCollect), data(data)
{
    ui->setupUi(this);
    connect(&auto_read_timer, &QTimer::timeout, this, &PageWidgetsCollect::slot_auto_read_timeout);
    is_auto_read = ui->checkBox->isChecked();
}

PageWidgetsCollect::~PageWidgetsCollect()
{
    delete ui;
}

void PageWidgetsCollect::showEvent(QShowEvent *event)
{
    (void)event;

    auto_read_start();
}
void PageWidgetsCollect::hideEvent(QHideEvent *event)
{
    (void)event;
    auto_read_stop();
}

void PageWidgetsCollect::auto_read_start()
{
    request_get_index = 0;
    auto_read_timer.start(100);
}

void PageWidgetsCollect::auto_read_stop()
{
    auto_read_timer.stop();
}

void PageWidgetsCollect::slot_auto_read_timeout()
{
    if (reg_map.count() < 1 || (request_get_index >= reg_map.count() && !is_auto_read))
    {
        auto_read_stop();
        return;
    }
    else
    {
        if (request_get_index >= reg_map.count())
        {
            request_get_index = 0;
        }
    }
    uint16_t reg_len          = 0;
    uint16_t request_reg_addr = reg_map.keys().at(request_get_index);
    int      id               = request_reg_addr;
    int      id_back          = id + 1;
    for (int i = request_get_index; i < reg_map.count(); i++)
    {
        id         = reg_map.keys().at(i);
        RegInfo *p = reg_map.value(id);
        if (qAbs(id - request_reg_addr) >= 64)
        {
            break;
        }
        request_get_index++;
        id_back = id + p->reg_len;
    }

    reg_len = qMin(qAbs(id_back - request_reg_addr), 64);
    emit sig_request_read_reg(request_reg_addr, reg_len);
}

void PageWidgetsCollect::json_items_handle(QJsonDocument *jdoc)
{
    auto       json_map = jdoc->toVariant().toJsonArray();
    QLayout   *layout   = ui->verticalLayout;
    QList<int> widget_height_list;
    // 解析json文件，并生成相应的界面
    for (auto widget_attr : json_map)
    {
        auto    obj       = widget_attr.toObject();
        QString page_type = obj.value("page_type").toString();
        if (page_type == "label_view")
        {
            QString node        = obj.value("node").toString();
            int     column_cnt  = obj.value("column_cnt").toInt();
            int     start_index = obj.value("start_index").toInt();
            int     end_index   = obj.value("end_index").toInt();

            MLabelTable *widget = new MLabelTable(node, column_cnt);
            for (int i = start_index; i <= end_index; i++)
            {
                if (data->obj_map.contains(i))
                {
                    // 获取数据实例，关联各种信号槽
                    DataObj *obj = data->obj_map.value(i);

                    MLabel *label = new MLabel(obj->name);                           // 创建带描述的只读控件
                    connect(obj, &DataObj::sig_update, label, &MLabel::slot_update); // 链接更新
                    widget->insert(label);

                    reg_map.insert(i, new RegInfo(i, obj->reg_len));
                }
            }
            layout->addWidget(widget);
            widget_height_list.append(widget->get_row_cnt());
        }
        else if (page_type == "table_view")
        {
            QString node          = obj.value("node").toString();
            int     start_index   = obj.value("start_index").toInt();
            int     row_cnt       = obj.value("row_cnt").toInt();
            int     column_cnt    = obj.value("column_cnt").toInt();
            QString row_header    = obj.value("row_header").toString();
            QString column_header = obj.value("column_header").toString();
            int     end_index     = start_index + row_cnt * column_cnt;

            MValueLabelTable *widget = new MValueLabelTable(node, row_cnt, column_cnt, row_header, column_header);
            for (int i = 0; i < row_cnt; i++)
            {
                int inc_len = 1;
                for (int j = 0; j < column_cnt; j++)
                {
                    int index = start_index + i * column_cnt + j * inc_len;
                    if (data->obj_map.contains(index))
                    {
                        // 获取数据实例，关联各种信号槽
                        DataObj     *obj   = data->obj_map.value(index);
                        MValueLabel *label = new MValueLabel();                               // 创建一个只读控件
                        connect(obj, &DataObj::sig_update, label, &MValueLabel::slot_update); // 链接更新
                        widget->insert(i, j, label);                                          // 插入可视表
                        inc_len = obj->reg_len;
                        reg_map.insert(index, new RegInfo(index, obj->reg_len));
                    }
                }
            }
            layout->addWidget(widget);
            widget_height_list.append(widget->get_row_cnt());
        }
        else if (page_type == "param_view")
        {
            QString node        = obj.value("node").toString();
            int     offset      = obj.value("offset").toInt(0);
            int     start_index = obj.value("start_index").toInt() + offset;
            int     end_index   = obj.value("end_index").toInt() + offset;

            MWriteReadTable *widget = new MWriteReadTable(node);
            for (int i = start_index; i <= end_index; i++)
            {
                if (data->obj_map.contains(i))
                {
                    // 获取数据实例，关联各种信号槽
                    DataObj          *obj = data->obj_map.value(i);
                    MWriteReadWidget *w = new MWriteReadWidget(obj->name, obj->min, obj->max, obj->def); // 创建一个读写控件
                    connect(obj, &DataObj::sig_update, w, &MWriteReadWidget::slot_update);               // 关联数据更新
                    connect(obj, &DataObj::sig_write_finish, w, &MWriteReadWidget::slot_update_finish);
                    connect(w, &MWriteReadWidget::sig_request_read, obj, &DataObj::slot_request_read_reg); // 链接请求读
                    connect(w, &MWriteReadWidget::sig_update, obj, &DataObj::slot_request_write_reg);      // 链接请求写
                    widget->insert(w);

                    reg_map.insert(i, new RegInfo(i, obj->reg_len));
                }
            }
            layout->addWidget(widget);
            widget_height_list.append(widget->get_row_cnt());
        }
    }
    for (int i = 0; i < widget_height_list.count(); i++)
    {
        ui->verticalLayout->setStretch(i, widget_height_list.at(i));
    }
}

void PageWidgetsCollect::on_checkBox_stateChanged(int arg1)
{
    (void)arg1;
    is_auto_read = ui->checkBox->isChecked();
    if (ui->checkBox->isChecked())
    {
        auto_read_start();
    }
    else
    {
        auto_read_stop();
    }
}
