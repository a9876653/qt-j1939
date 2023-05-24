#ifndef DATAOBJMAP_H
#define DATAOBJMAP_H

#include <QObject>
#include <QTimer>
#include "dataobj.h"
#include "singleton.h"

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
    DataObjMap(int src_addr = 0xFF);
    ~DataObjMap();

    void json_items_handle(QJsonDocument *jdoc);
    bool load_json(QString path);
    bool save_write_data_json(QString path);
    bool load_write_data_json(QString path);

    void save_read_data_file();

public:
    QMap<int, DataObj *> obj_map;
    QMap<int, DataObj *> param_map;

private:
    QMap<int, JsonStruct *> json_struct_map;
    int                     src_addr;

    QTimer      csv_timer;
    QStringList csv_header;
    QString     csv_path;
};
#define DataObjMapIns Singleton<DataObjMap>::getInstance()
#endif // DATAOBJMAP_H
