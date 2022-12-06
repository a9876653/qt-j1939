#include "msgdata.h"
#include "canard_dsdl.h"

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

        sig->rec_dec_widget.text->setText(dec_s);
        sig->rec_hex_widget.text->setText(dec_s);
    }
}
