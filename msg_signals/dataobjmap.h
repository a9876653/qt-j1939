#ifndef DATAOBJMAP_H
#define DATAOBJMAP_H

#include <QObject>
#include "dataobj.h"

class JsonStruct
{
public:
    JsonStruct(QString name, int id, double def_v, bool is_array, bool is_write, int array_size, QString type)
        : name(name), id(id), def_v(def_v), is_array(is_array), is_write(is_write), array_size(array_size), type(type)
    {
    }

public:
    QString name;
    int     id;
    double  def_v;
    bool    is_array;
    bool    is_write;
    int     array_size;
    QString type;
};

class DataObjMap : public QObject
{
public:
    DataObjMap(int src_addr);

    void json_items_handle(QJsonDocument *jdoc);
    bool load_json(QString path);

public:
    QMap<int, DataObj *> obj_map;

private:
    QMap<int, JsonStruct *> json_struct_map;
    int                     src_addr;
};

#endif // DATAOBJMAP_H