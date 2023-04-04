#include "dataobjmap.h"
#include "filehandle.h"
#include "canard_dsdl.h"
#include "j1939_modbus_master.h"
#include "QDebug"
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
        auto        obj        = param.toObject();
        QString     name       = obj.value("note").toString();
        int         id         = obj.value("id").toInt();
        double      def_v      = obj.value("def_value").toDouble();
        bool        is_array   = obj.value("is_array").toBool();
        bool        is_write   = obj.value("is_write").toBool();
        int         array_size = obj.value("array_size").toInt();
        int         max        = obj.value("max").toInt();
        int         min        = obj.value("min").toInt();
        QString     type       = obj.value("type").toString();
        int         reg_num    = 1;
        JsonStruct *node       = new JsonStruct(name, id, def_v, is_array, is_write, array_size, type);
        json_struct_map.insert(id, node);
        if (type.contains("32"))
        {
            reg_num = 2;
        }
        for (int i = 0; i < array_size; i++)
        {
            QString param_name = name;
            if (is_array)
            {
                param_name = name + QString("-%1").arg(i);
            }
            int          param_id = id + reg_num * i;
            DataObj     *p        = new DataObj(param_name, param_id, type, min, max, def_v);
            CommDbValue *v        = J1939DbIns->db_reg_register(src_addr, param_id, reg_num);

            connect(p, &DataObj::sig_request_read_reg, J1939DbIns, &CommJ1939Db::slot_request_read_reg);
            connect(v, &CommDbValue::sig_read_finish, p, &DataObj::slot_read_finish);
            if (is_write)
            {
                connect(p, &DataObj::sig_request_write_reg, J1939DbIns, &CommJ1939Db::slot_request_write_reg);
                connect(v, &CommDbValue::sig_write_finish, p, &DataObj::slot_write_finish);
            }
            obj_map.insert(param_id, p);
        }
    }
}
