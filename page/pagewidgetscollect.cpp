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

bool PageWidgetsCollect::auto_read_en = true;

PageWidgetsCollect::PageWidgetsCollect(DataObjMap *data) : ui(new Ui::PageWidgetsCollect), data(data)
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
    slot_auto_read_timeout();
}
void PageWidgetsCollect::hideEvent(QHideEvent *event)
{
    (void)event;
    // auto_read_stop();
}

void PageWidgetsCollect::auto_read_start()
{
    request_get_index = 0;
    auto_read_timer.stop();
    auto_read_timer.start(update_period);
}

void PageWidgetsCollect::auto_read_stop()
{
    auto_read_timer.stop();
}

void PageWidgetsCollect::slot_auto_read_timeout()
{
    uint16_t map_count = reg_info_list.count();
    if (!auto_read_en && is_auto_read) // 禁止自动读
    {
        return;
    }
    if (map_count < 1 || (request_get_index >= map_count && !is_auto_read))
    {
        auto_read_stop();
        return;
    }
    else
    {
        if (request_get_index >= reg_info_list.count())
        {
            request_get_index = 0;
        }
    }
    while (request_get_index < map_count)
    {
        uint16_t reg_len          = 0;
        RegInfo  p                = reg_info_list.at(request_get_index);
        uint16_t request_reg_addr = p.reg_addr;
        int      id               = request_reg_addr;
        int      id_back          = id + 1;
        for (int i = request_get_index; i < map_count; i++)
        {
            RegInfo p = reg_info_list.at(i);
            id        = p.reg_addr;
            if (qAbs(id - request_reg_addr) >= 64)
            {
                break;
            }
            request_get_index++;
            id_back = id + p.reg_len;
        }

        reg_len = qMin(qAbs(id_back - request_reg_addr), 64);
        emit sig_request_read_reg(request_reg_addr, reg_len);
    }
}

void PageWidgetsCollect::addpend_obj(int index, DataObj *obj)
{
    if (is_save)
    {
        save_obj_list.append(obj);
    }
    reg_info_list.append(RegInfo(index, obj->reg_len));
    reg_map.insert(index, new RegInfo(index, obj->reg_len));
}

void PageWidgetsCollect::json_items_handle(QJsonDocument *jdoc)
{
    auto       json_map = jdoc->toVariant().toMap();
    QLayout   *layout   = ui->verticalLayout;
    QList<int> widget_height_list;
    QString    is_auto_read_key   = "is_auto_read";
    QString    read_period_ms_key = "read_period_ms";
    QString    is_save_key        = "is_save";
    if (json_map.contains(is_auto_read_key))
    {
        is_auto_read = json_map.value(is_auto_read_key).toBool();
    }
    else
    {
        is_auto_read = false;
    }
    if (json_map.contains(read_period_ms_key))
    {
        update_period = json_map.value(read_period_ms_key).toInt();
    }
    else
    {
        update_period = 1000;
    }
    if (json_map.contains(is_save_key))
    {
        is_save = json_map.value(is_save_key).toInt();
    }
    else
    {
        is_save = false;
    }
    ui->updatePeriodSpinBox->setValue(update_period);
    auto pages_map = json_map.value("pages").toJsonArray();
    // 解析json文件，并生成相应的界面
    for (auto widget_attr : pages_map)
    {
        auto    obj         = widget_attr.toObject();
        QString page_type   = obj.value("page_type").toString();
        QString node        = obj.value("node").toString();
        int     offset      = obj.value("offset").toInt(0);
        int     start_index = obj.value("start_index").toInt() + offset;
        int     reg_num     = obj.value("reg_num").toInt();
        int     end_index   = start_index + reg_num;
        if (obj.contains("end_index"))
        {
            end_index = obj.value("end_index").toInt() + offset;
        }

        if (page_type == "label_view")
        {
            int column_cnt = obj.value("column_cnt").toInt();

            MLabelTable *widget = new MLabelTable(node, column_cnt);
            for (int i = start_index; i <= end_index; i++)
            {
                if (data->obj_map.contains(i))
                {
                    // 获取数据实例，关联各种信号槽
                    DataObj *obj = data->obj_map.value(i);

                    MLabel *label = new MLabel(obj->name, obj->value_des);           // 创建带描述的只读控件
                    connect(obj, &DataObj::sig_update, label, &MLabel::slot_update); // 链接更新
                    widget->insert(label);

                    addpend_obj(i, obj);
                }
            }
            layout->addWidget(widget);
            widget_height_list.append(widget->get_row_cnt());
        }
        else if (page_type == "table_view")
        {
            int     row_cnt       = obj.value("row_cnt").toInt();
            int     column_cnt    = obj.value("column_cnt").toInt();
            QString row_header    = obj.value("row_header").toString();
            QString column_header = obj.value("column_header").toString();

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
                        addpend_obj(index, obj);
                    }
                }
            }
            layout->addWidget(widget);
            widget_height_list.append(widget->get_row_cnt());
        }
        else if (page_type == "param_view")
        {
            MWriteReadTable *widget = new MWriteReadTable(node);
            for (int i = start_index; i <= end_index; i++)
            {
                if (data->obj_map.contains(i))
                {
                    // 获取数据实例，关联各种信号槽
                    DataObj          *obj = data->obj_map.value(i);
                    MWriteReadWidget *w
                        = new MWriteReadWidget(obj->name, obj->min, obj->max, obj->def, obj->write_value); // 创建一个读写控件
                    connect(obj, &DataObj::sig_update, w, &MWriteReadWidget::slot_update);                 // 关联数据更新
                    connect(obj, &DataObj::sig_write_finish, w, &MWriteReadWidget::slot_update_finish);
                    connect(w, &MWriteReadWidget::sig_request_read, obj, &DataObj::slot_request_read_reg); // 链接请求读
                    connect(w, &MWriteReadWidget::sig_update, obj, &DataObj::slot_request_write_reg);      // 链接请求写
                    widget->insert(w);

                    addpend_obj(i, obj);
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
    ui->checkBox->setChecked(is_auto_read);
    on_checkBox_stateChanged(is_auto_read);
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

void PageWidgetsCollect::on_updatePeriodSpinBox_valueChanged(int arg1)
{
    update_period = ui->updatePeriodSpinBox->value();
    on_checkBox_stateChanged(arg1);
}
