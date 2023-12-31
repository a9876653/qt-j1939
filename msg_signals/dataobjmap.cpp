#include "dataobjmap.h"
#include "QDebug"
#include "canard_dsdl.h"
#include "filehandle.h"
#include "j1939_modbus_master.h"
#include "json_file.h"
#define DATAMAP_DBG(x...) qDebug(x)

DataObjMap::DataObjMap(int src_addr) : src_addr(src_addr)
{
    QString       dir      = "./cfg/regs_json";
    QFileInfoList fileList = GetFileList(dir); //获取目录下所有的文件
    for (QFileInfo info : fileList)
    {
        DATAMAP_DBG() << "file path:" << info.filePath() << "   file name:" << info.fileName();
        if (info.filePath().contains(".json")) // 只处理JSON文件
        {
            load_json(info.filePath());
        }
    }
    load_write_data_json("./temp/obj_temp.json");

#if 0
    csv_header.append("时间");
    csv_header.append("时间UTC");
    for (DataObj *obj : obj_map)
    {
        csv_header.append(obj->name_en);
    }

    connect(&csv_timer, &QTimer::timeout, this, &DataObjMap::save_read_data_file);
    csv_timer.start(1000);
#endif
}

DataObjMap::~DataObjMap()
{
    save_write_data_json("./temp/obj_temp.json");
}

void DataObjMap::save_read_data_file()
{
    QList<QString> data;
    QDateTime      time;
    QDate          time_data = time.currentDateTime().date();
    QString        time_s    = QString("%1%2%3")
                         .arg(time_data.year())
                         .arg(time_data.month(), 2, 10, QChar('0'))
                         .arg(time_data.day(), 2, 10, QChar('0'));
    csv_path = QString("./save_data/db-%1-%2.csv").arg(time_s).arg(src_addr);
    data.append(get_current_time_ms_qstring());
    data.append(QString("%1").arg(sys_get_ms()));

    for (DataObj *obj : obj_map)
    {
        data.append(QString("%1").arg(obj->read_value.toString()));
    }

    CsvAppend(csv_path, csv_header, data);
}

bool DataObjMap::save_write_data_json(QString path)
{
    QJsonObject root;
    for (DataObj *obj : obj_map)
    {
        root.insert(QString("%1").arg(obj->id), obj->write_value.toJsonValue());
    }
    QJsonDocument tempJdoc(root);
    write_json_file(path, &tempJdoc);
    return true;
}

bool DataObjMap::load_write_data_json(QString path)
{
    QJsonObject root;
    if (load_json_file(path, &root))
    {
        QVariantMap map = root.toVariantMap();
        for (QString key : map.keys())
        {
            bool ok = false;
            int  id = key.toInt(&ok);
            if (ok)
            {
                if (obj_map.contains(id))
                {
                    obj_map.value(id)->write_value = map.value(key).toInt();
                }
            }
        }
    }
    return true;
}

bool DataObjMap::load_json(QString path)
{
    QJsonDocument jsonCfgDoc;
    QFile         file_read(path);
    QFileInfo     fileInfo = QFileInfo(path);

    if (!file_read.open(QIODevice::ReadOnly))
    {
        DATAMAP_DBG("read json file failed");
        return false;
    }
    QByteArray read_array = file_read.readAll();
    file_read.close();

    QJsonParseError e;
    jsonCfgDoc = QJsonDocument::fromJson(read_array, &e);

    if (e.error != QJsonParseError::NoError && !jsonCfgDoc.isNull())
    {
        DATAMAP_DBG("read json file error %d", e.error);
        return false;
    }
    json_items_handle(&jsonCfgDoc);
    return true;
}

void DataObjMap::json_items_handle(QJsonDocument *jdoc)
{
    auto json_map = jdoc->toVariant().toJsonArray();

    for (auto param : json_map)
    {
        QJsonObject obj        = param.toObject();
        QString     name       = obj.value("note").toString();
        QString     name_en    = obj.value("name").toString();
        QString     unit       = obj.value("unit").toString();
        int         id         = obj.value("id").toInt();
        double      def_v      = obj.value("def_value").toDouble();
        bool        is_array   = obj.value("is_array").toBool();
        bool        is_write   = obj.value("is_write").toBool();
        int         array_size = obj.value("array_size").toInt();
        QString     type       = obj.value("type").toString();
        double      max        = obj.value("max").toDouble();
        double      min        = obj.value("min").toDouble();
        int         param_id   = obj.value("param_id").toInt();
        if (unit != "")
        {
            name += QString("(%1)").arg(unit);
        }
        ValueDes value_des;
        if (obj.contains("value_des"))
        {
            QJsonObject value_des_obj = obj.value("value_des").toObject();
            value_des.is_bit_des      = value_des_obj.value("is_bit_des").toBool(false);
            QJsonObject des_map       = value_des_obj.value("value_des_enum").toObject();
            for (QString key : des_map.keys())
            {
                QVariant k = key.toUInt();
                value_des.value_des_map.insert(k, des_map.value(key).toString());
            }
        }

        int         reg_num = 1;
        JsonStruct *node    = new JsonStruct(name, id, def_v, is_array, is_write, array_size, type);
        json_struct_map.insert(id, node);
        if (type.contains("32"))
        {
            reg_num = 2;
        }
        for (int i = 0; i < array_size; i++)
        {
            QString param_name    = name;
            QString param_name_en = name_en;
            if (is_array)
            {
                param_name    = name + QString("-%1").arg(i + 1);
                param_name_en = name_en + QString("-%1").arg(i + 1);
            }
            int          msg_id = id + reg_num * i;
            DataObj     *p      = new DataObj(param_name, param_name_en, msg_id, type, min, max, def_v, value_des);
            CommDbValue *v      = J1939DbIns->db_reg_register(src_addr, msg_id, reg_num);

            connect(p, &DataObj::sig_request_read_reg, J1939DbIns, &CommJ1939Db::slot_request_read_reg);
            connect(v, &CommDbValue::sig_read_finish, p, &DataObj::slot_read_finish);
            if (is_write)
            {
                connect(p, &DataObj::sig_request_write_reg, J1939DbIns, &CommJ1939Db::slot_request_write_reg);
                connect(v, &CommDbValue::sig_write_finish, p, &DataObj::slot_write_finish);
            }
            obj_map.insert(msg_id, p);
            if (param_id >= 0)
            {
                param_map.insert(param_id, p);
            }
        }
    }
}
