#include "msg_signals.h"
#include "QDebug"
#include "QFile"

#define MSG_SIGNAL_DBG(x...) qDebug(x)

QFileInfoList GetFileList(QString path)
{
    QDir          dir(path);
    QFileInfoList file_list   = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (int i = 0; i != folder_list.size(); i++)
    {
        QString       name            = folder_list.at(i).absoluteFilePath();
        QFileInfoList child_file_list = GetFileList(name);
        file_list.append(child_file_list);
    }

    return file_list;
}

MsgSignals::MsgSignals()
{
    QString       dir      = "./cfg/";
    QFileInfoList fileList = GetFileList(dir); //获取目录下所有的文件
    for (QFileInfo info : fileList)
    {
        MSG_SIGNAL_DBG() << "file path:" << info.filePath() << "   file name:" << info.fileName();
        if (info.filePath().contains(".json")) // 只处理JSON文件
        {
            load_json(info.filePath());
        }
    }
}

bool MsgSignals::load_json(QString path)
{
    QJsonDocument jsonCfgDoc;
    QFile         file_read(path);
    if (!file_read.open(QIODevice::ReadOnly))
    {
        MSG_SIGNAL_DBG("read json file failed");
        return false;
    }
    QByteArray read_array = file_read.readAll();
    file_read.close();

    QJsonParseError e;
    jsonCfgDoc = QJsonDocument::fromJson(read_array, &e);

    if (e.error != QJsonParseError::NoError && !jsonCfgDoc.isNull())
    {
        MSG_SIGNAL_DBG("read json file error %d", e.error);
        return false;
    }
    json_items_handle(&jsonCfgDoc);
    return true;
}

void MsgSignals::json_items_handle(QJsonDocument *jdoc)
{
    auto       msg_map   = jdoc->object().toVariantMap();
    QJsonArray msg_array = msg_map.value("messages").toJsonArray();

    for (const auto &msg_obj : msg_array)
    {
        MsgData *msg_data    = new MsgData;
        uint16_t msg_bit_len = 0;
        auto     msg         = msg_obj.toVariant().toMap();
        msg_data->id         = msg.value("id").toUInt();
        msg_data->name       = msg.value("name").toString();
        msg_data->pgn        = (msg_data->id & 0xFFFF00) >> 8;
        auto sig_array       = msg.value("signals").toJsonArray();
        for (const auto &sig_obj : sig_array)
        {
            Signal *msg_sig = new Signal();

            auto sig            = sig_obj.toVariant().toMap();
            msg_sig->name       = sig.value("name").toString();
            msg_sig->start_bit  = sig.value("start_bit").toUInt();
            msg_sig->bit_length = sig.value("bit_length").toUInt();
            msg_sig->is_float   = sig.value("is_float").toBool();
            msg_sig->is_signed  = sig.value("is_signed").toBool();
            msg_bit_len += msg_sig->bit_length;

            connect(&msg_sig->send_widget, &CustomTextWidget::sig_enter_event, msg_data, &MsgData::slot_encode_send);

            msg_data->signals_list.append(msg_sig);
        }
        msg_data->msg_len = msg_bit_len / 8;
        if (msg_bit_len % 8 != 0)
        {
            msg_data->msg_len++;
        }
        connect(msg_data, &MsgData::sig_msg_send, J1939Ins, &CommJ1939::slot_msg_send);

        msgs_map.insert(msg_data->pgn, msg_data);
    }
    MSG_SIGNAL_DBG("json_items_handle finish");
}
