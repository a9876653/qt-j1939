#include "msg_signals.h"
#include "QDebug"
#include "QFile"
#include "filehandle.h"
#include "json_file.h"

#define MSG_SIGNAL_DBG(x...) qDebug(x)

MsgSignals::MsgSignals(bool is_master) : is_master(is_master)
{
    QString       dir      = "./cfg/can_dbc";
    QFileInfoList fileList = GetFileList(dir); //获取目录下所有的文件
    for (QFileInfo info : fileList)
    {
        MSG_SIGNAL_DBG() << "file path:" << info.filePath() << "   file name:" << info.fileName();
        if (info.filePath().contains(".json")) // 只处理JSON文件
        {
            load_json(info.filePath());
        }
    }

    if (is_master)
    {
        load_temp_file("./temp/msg_temp.json");
    }
}

MsgSignals::~MsgSignals()
{
    save_msg_send_data("./temp/msg_temp.json");
    MSG_SIGNAL_DBG() << "msg signals save send data";
}

bool MsgSignals::load_json(QString path)
{
    QJsonDocument jsonCfgDoc;
    QFile         file_read(path);
    QFileInfo     fileInfo = QFileInfo(path);
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
    QMap<uint, MsgData *> obj_map;
    json_items_handle(&jsonCfgDoc, obj_map);
    file_msg_map.insert(fileInfo.fileName(), obj_map);
    return true;
}

void MsgSignals::json_items_handle(QJsonDocument *jdoc, QMap<uint, MsgData *> &obj_map)
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
            if (is_master)
            {
                connect(&msg_sig->send_widget, &CustomTextWidget::sig_enter_event, this, &MsgSignals::slot_msg_send);
            }

            msg_data->signals_list.append(msg_sig);
        }
        msg_data->msg_len = msg_bit_len / 8;
        if (msg_bit_len % 8 != 0)
        {
            msg_data->msg_len++;
        }
        connect(msg_data, &MsgData::sig_msg_send, J1939Ins, &CommJ1939::slot_msg_send);
        connect(msg_data, &MsgData::sig_request_pgn, J1939Ins, &CommJ1939::slot_request_pgn);

        obj_map.insert(msg_data->pgn, msg_data);
        msgs_map.insert(msg_data->pgn, msg_data);
    }
    MSG_SIGNAL_DBG("json_items_handle finish");
}

bool MsgSignals::load_temp_file(QString path)
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
    QJsonObject root = jsonCfgDoc.toVariant().toJsonObject();
    for (auto root_item : root)
    {
        QJsonObject msg_obj = root_item.toObject();
        uint32_t    pgn     = msg_obj.value("pgn").toString().toInt();
        if (msgs_map.contains(pgn))
        {
            MsgData *msg_data = msgs_map.value(pgn);
            if (msg_obj.contains("signals"))
            {
                QJsonObject signals_obj = msg_obj.value("signals").toObject();
                for (Signal *sig : msg_data->signals_list)
                {
                    if (signals_obj.contains(sig->name))
                    {
                        QJsonObject sig_obj = signals_obj.value(sig->name).toObject();
                        sig->send_widget.text->setText(sig_obj.value("value").toString());
                    }
                }
            }
        }
    }
    return true;
}

void MsgSignals::slot_msg_send()
{
}

void MsgSignals::save_msg_send_data(QString path)
{
    QJsonObject root;
    for (MsgData *msg_data : msgs_map)
    {
        QJsonObject msg_obj;
        msg_obj.insert("name", msg_data->name);
        msg_obj.insert("id", QString("%1").arg(msg_data->id));
        msg_obj.insert("pgn", QString("%1").arg(msg_data->pgn));
        QJsonObject signals_obj;
        for (Signal *sig : msg_data->signals_list)
        {
            QJsonObject sig_obj;
            sig_obj.insert("value", sig->send_widget.text->toPlainText());
            signals_obj.insert(sig->name, sig_obj);
        }
        msg_obj.insert("signals", signals_obj);
        root.insert(QString("%1").arg(msg_data->pgn), msg_obj);
    }
    QJsonDocument tempJdoc(root);
    write_json_file(path, &tempJdoc);
}
