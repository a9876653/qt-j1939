#include "paramshandle.h"
#include "QDebug"
#include "filehandle.h"
#include "canard_dsdl.h"

#define PARAM_DBG(x...) qDebug(x)

ParamsHandle::ParamsHandle()
{
    QString       dir      = "./cfg/param";
    QFileInfoList fileList = GetFileList(dir); //获取目录下所有的文件
    for (QFileInfo info : fileList)
    {
        PARAM_DBG() << "file path:" << info.filePath() << "   file name:" << info.fileName();
        if (info.filePath().contains(".json")) // 只处理JSON文件
        {
            load_json(info.filePath());
        }
    }
}

bool ParamsHandle::load_json(QString path)
{
    QJsonDocument jsonCfgDoc;
    QFile         file_read(path);
    QFileInfo     fileInfo = QFileInfo(path);
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
    param_vector.clear();
    auto param_map = jdoc->toVariant().toJsonArray();

    for (auto param : param_map)
    {
        auto       obj   = param.toObject();
        QString    name  = obj.value("name").toString();
        int        id    = obj.value("id").toInt();
        double     def_v = obj.value("def_value").toDouble();
        ParamData *p     = new ParamData(id, name, def_v);
        connect(p, &ParamData::sig_msg_send, J1939Ins, &CommJ1939::slot_msg_send);
        param_vector.append(p);
    }
}

void ParamsHandle::decode(uint8_t *data, uint16_t data_size)
{
    uint id    = canardDSDLGetU32((const uint8_t *)data, data_size, 0, 32);
    int  value = canardDSDLGetU32((const uint8_t *)data, data_size, 32, 32);
    if (id < param_vector.count())
    {
        param_vector[id]->set_recv_value(value);
    }
}
