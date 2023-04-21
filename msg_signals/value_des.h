#ifndef VALUE_DES_H
#define VALUE_DES_H

#include <QObject>
#include <QVariant>

class ValueDes
{
public:
    QMap<QVariant, QString> value_des_map = {};
    bool                    is_bit_des    = false;
};

#endif // VALUE_DES_H
