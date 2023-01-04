#ifndef PARAMSHANDLE_H
#define PARAMSHANDLE_H

#include "QObject"
#include "paramdata.h"

class ParamsHandle : public QObject
{
    Q_OBJECT
public:
    ParamsHandle();

    void decode(uint8_t *data, uint16_t data_size);

    QVector<ParamData *> param_vector;

private:
    bool load_json(QString path);
    void json_items_handle(QJsonDocument *jdoc);
};

#endif // PARAMSHANDLE_H
