#ifndef FILEHANDLE_H
#define FILEHANDLE_H

#include <QObject>
#include <QtCore>
#include "QFile"

QFileInfoList GetFileList(QString path);

bool CsvAppend(QString path, QList<QString> header, QList<QString> data);

#endif // FILEHANDLE_H
