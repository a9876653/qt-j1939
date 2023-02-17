#include "paramshandle.h"
#include "QDebug"
#include "filehandle.h"
#include "canard_dsdl.h"
#include "j1939_modbus_master.h"

#define PARAM_DBG(x...) qDebug(x)

ParamsHandle::ParamsHandle(int src_addr) : src_addr(src_addr)
{
    connect(&request_get_timer, &QTimer::timeout, this, &ParamsHandle::slot_request_get_timeout);
    request_get_timer.setInterval(1000);
    request_get_timer.stop();

    connect(&request_set_timer, &QTimer::timeout, this, &ParamsHandle::slot_request_set_timeout);
    request_set_timer.setInterval(10);
    request_set_timer.stop();

    connect(J1939DbIns, &CommJ1939Db::sig_recv_read_reg, this, &ParamsHandle::slot_respond_read_reg);
}

bool ParamsHandle::load_json(QString _path)
{
    QJsonDocument jsonCfgDoc;
    path = _path;
    QFile     file_read(path);
    QFileInfo fileInfo = QFileInfo(path);

    if (!file_read.open(QIODevice::ReadOnly))
    {
        PARAM_DBG("read json file failed");
        return false;
    }
    QByteArray read_array = file_read.readAll();
    file_read.close();

    QJsonParseError e;
    jsonCfgDoc = QJsonDocument::fromJson(read_array, &e);

    if (e.error != QJsonParseError::NoError && !jsonCfgDoc.isNull())
    {
        PARAM_DBG("read json file error %d", e.error);
        return false;
    }
    json_items_handle(&jsonCfgDoc);
    return true;
}

void ParamsHandle::json_items_handle(QJsonDocument *jdoc)
{
    param_map.clear();
    auto json_map = jdoc->toVariant().toJsonArray();

    for (auto param : json_map)
    {
        auto    obj         = param.toObject();
        QString name        = obj.value("note").toString();
        int     id          = obj.value("id").toInt();
        double  def_v       = obj.value("def_value").toDouble();
        bool    is_array    = obj.value("is_array").toBool();
        bool    is_write    = obj.value("is_write").toBool();
        int     array_size  = obj.value("array_size").toInt();
        QString type        = obj.value("type").toString();
        int     inc_reg_num = 1;
        if (type.contains("32"))
        {
            inc_reg_num = 2;
        }
        for (int i = 0; i < array_size; i++)
        {
            int param_id = id + inc_reg_num * i;

            QString param_name = name;
            if (is_array)
            {
                param_name = name + QString("-%1").arg(i);
            }
            ParamData   *p = new ParamData(param_id, param_name, type, def_v);
            CommDbValue *v = J1939DbIns->db_reg_register(src_addr, param_id, inc_reg_num);

            connect(p, &ParamData::sig_request_read_reg, J1939DbIns, &CommJ1939Db::slot_request_read_reg);
            connect(v, &CommDbValue::sig_read_finish, p, &ParamData::slot_read_finish);
            if (is_write)
            {
                connect(p, &ParamData::sig_request_write_reg, J1939DbIns, &CommJ1939Db::slot_request_write_reg);
                connect(v, &CommDbValue::sig_write_finish, p, &ParamData::slot_write_finish);
            }
            else
            {
                p->send_widget.line_edit->setDisabled(true);
            }

            param_map.insert(param_id, p);
        }
    }
}

void ParamsHandle::auto_get_param_table(bool en)
{
    is_auto_get = en;
    if (is_auto_get)
    {
        request_get_index = 0;
        request_get_timer.start();
        request_get_regs();
    }
    else
    {
        request_get_timer.stop();
    }
}

void ParamsHandle::set_param_table()
{
    request_set_index = 0;
    request_set_timer.start();
    PARAM_DBG("开始设置参数列表");
    slot_request_set_timeout();
}

void ParamsHandle::get_param_table()
{
    request_get_index = 0;
    request_get_timer.start();
    PARAM_DBG("开始请求参数列表");
    request_get_regs();
}

void ParamsHandle::slot_request_set_timeout()
{
    if (request_set_index >= param_map.count())
    {
        request_set_timer.stop();
        PARAM_DBG("请求设置参数列表完成");
    }
    else
    {
        int        id = param_map.keys().at(request_set_index);
        ParamData *p  = param_map.value(id);
        p->slot_request_write();
        request_set_index++;
    }
}

void ParamsHandle::slot_request_get_timeout()
{
    request_get_regs();
}
void ParamsHandle::request_get_regs()
{
    if (request_get_index >= param_map.count() && !is_auto_get)
    {
        request_get_timer.stop();
        PARAM_DBG("请求参数列表完成");
        return;
    }
    else
    {
        if (request_get_index >= param_map.count())
        {
            request_get_index = 0;
        }
    }
    uint16_t reg_len = 0;
    request_reg_addr = param_map.keys().at(request_get_index);
    int id           = request_reg_addr;
    int id_back      = id + 1;
    for (int i = request_get_index; i < param_map.count(); i++)
    {
        id = param_map.keys().at(i);

        ParamData *param = param_map.value(id);
        if (qAbs(id - request_reg_addr) >= 64)
        {
            break;
        }
        param->slot_set_icon(false);
        request_get_index++;
        id_back = id + param->reg_len;
    }

    reg_len = qMin(qAbs(id_back - request_reg_addr), 64);
    J1939DbIns->slot_request_read_reg(request_reg_addr, reg_len);
}

void ParamsHandle::slot_respond_read_reg(uint16_t reg_addr, uint8_t data_len, uint8_t *data)
{
    (void)data_len;
    (void)data;
    if (reg_addr == request_reg_addr)
    {
        request_get_regs();
    }
}
