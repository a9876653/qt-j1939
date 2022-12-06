#include "msg_signals.h"
#include "QDebug"
#include "QFile"

#define MSG_SIGNAL_DBG(x...) qDebug(x)

MsgSignals::MsgSignals()
{
}

bool MsgSignals::load_json(QString path, QString name)
{
    QJsonDocument jsonCfgDoc;
    QFile         file_read(path + name);
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
        MsgData msg_data;
        // qDebug() << msg;
        auto msg       = msg_obj.toVariant().toMap();
        msg_data.id    = msg.value("id").toUInt();
        msg_data.name  = msg.value("name").toString();
        msg_data.pgn   = (msg_data.id & 0xFFFF00) >> 8;
        auto sig_array = msg.value("signals").toJsonArray();
        for (const auto &sig_obj : sig_array)
        {
            MsgData::Signal *msg_sig = new MsgData::Signal();

            auto sig            = sig_obj.toVariant().toMap();
            msg_sig->name       = sig.value("name").toString();
            msg_sig->start_bit  = sig.value("start_bit").toUInt();
            msg_sig->bit_length = sig.value("bit_length").toUInt();
            msg_sig->is_float   = sig.value("is_float").toBool();
            msg_sig->is_signed  = sig.value("is_signed").toBool();
            msg_data.signals_list.append(msg_sig);
        }
        msgs_map.insert(msg_data.id, msg_data);
    }
    MSG_SIGNAL_DBG("json_items_handle finish");
}
