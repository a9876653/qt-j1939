#include "json_file.h"
#include "QDebug"
#include "QDir"
#include "QFileInfo"

#define JSON_FILE_DBG(x...) qDebug(x)

bool check_and_create_dir(QString path)
{
    QFileInfo file_info(path);
    if (file_info.exists())
    {
        return true;
    }
    else
    {
        QDir dir;
        bool ok = dir.mkpath(file_info.absolutePath()); //创建多级目录
        JSON_FILE_DBG("make dir %s", file_info.absolutePath().toLatin1().data());
        return ok;
    }
}

void write_json_file(QString path, QJsonDocument *jdoc)
{
    check_and_create_dir(path);
    QFile file_write(path);
    if (!file_write.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "write json file failed";
        return;
    }
    QByteArray read_array = jdoc->toJson();
    file_write.write(read_array);
    file_write.close();
}

bool load_json_file(QString path, QJsonObject *root)
{
    QJsonDocument jsonCfgDoc;
    QFile         file_read(path);
    if (!file_read.open(QIODevice::ReadOnly))
    {
        JSON_FILE_DBG("read json file failed");
        return false;
    }
    QByteArray read_array = file_read.readAll();
    file_read.close();

    QJsonParseError e;
    jsonCfgDoc = QJsonDocument::fromJson(read_array, &e);

    if (e.error != QJsonParseError::NoError && !jsonCfgDoc.isNull())
    {
        JSON_FILE_DBG("read json file error %d", e.error);
        return false;
    }
    *root = jsonCfgDoc.toVariant().toJsonObject();

    return true;
}
