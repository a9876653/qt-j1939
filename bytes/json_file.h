#ifndef JSON_FILE_H
#define JSON_FILE_H

#include "QFile"
#include "QJsonDocument"
#include "QJsonObject"

void write_json_file(QString path, QJsonDocument *jdoc);
bool load_json_file(QString path, QJsonObject *root);

#endif // JSON_FILE_H
