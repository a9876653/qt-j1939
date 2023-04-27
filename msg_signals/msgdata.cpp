#include "msgdata.h"
#include "canard_dsdl.h"

#include "QDebug"

#define MSGDATA_DBG(x...) qDebug(x)

void MsgData::slot_encode_send()
{
    MSGDATA_DBG("消息发送, Name:%s, Pgn: 0x%04x,%d\n\tSignals:", name.toLatin1().data(), pgn, pgn & 0xFFF);
    for (Signal *sig : signals_list)
    {
        QString dec_s = sig->send_widget.text->toPlainText();
        MSGDATA_DBG("\t\t%s: %s", sig->name.toLatin1().data(), dec_s.toLatin1().data());
        if (sig->is_float)
        {
            float value = dec_s.toFloat();
            canardDSDLSetF32(encode_buff, sig->start_bit, value);
        }
        else if (sig->is_signed)
        {
            int value = dec_s.toInt();
            canardDSDLSetIxx(encode_buff, sig->start_bit, value, sig->bit_length);
        }
        else
        {
            uint value = dec_s.toUInt();
            canardDSDLSetUxx(encode_buff, sig->start_bit, value, sig->bit_length);
        }
    }

    emit sig_msg_send(pgn, encode_buff, msg_len);
}

void MsgData::slot_request_pgn(uint8_t src)
{
    MSGDATA_DBG("请求PGN:0x%04x, dst:%d, len:%d", pgn, src, msg_len);
    emit sig_request_pgn(pgn, src, msg_len);
}

void MsgData::decode(uint8_t *data, uint16_t data_size)
{
    for (Signal *sig : signals_list)
    {
        QString dec_s, hex_s;
        sig->value_f = canardDSDLGetF32((const uint8_t *)data, data_size, sig->start_bit);
        sig->value_u = canardDSDLGetU32((const uint8_t *)data, data_size, sig->start_bit, sig->bit_length);
        sig->value_i = canardDSDLGetI32((const uint8_t *)data, data_size, sig->start_bit, sig->bit_length);
        if (sig->is_float)
        {
            dec_s = QString("%1").arg(sig->value_f);
            hex_s = QString("%1").arg(sig->value_u, 8, 16, QLatin1Char('0'));
        }
        else if (sig->is_signed)
        {
            dec_s = QString("%1").arg(sig->value_i);
            hex_s = QString("%1").arg(sig->value_i, 8, 16, QLatin1Char('0'));
        }
        else
        {
            dec_s = QString("%1").arg(sig->value_u);
            hex_s = QString("%1").arg(sig->value_u, 8, 16, QLatin1Char('0'));
        }
        sig->value_s = dec_s;
        sig->rec_dec_widget.text->setText(dec_s);
        sig->rec_hex_widget.text->setText(hex_s);
    }
}
