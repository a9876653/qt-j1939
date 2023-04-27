#include "filehandle.h"

QFileInfoList GetFileList(QString path)
{
    QDir          dir(path);
    QFileInfoList file_list   = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (int i = 0; i != folder_list.size(); i++)
    {
        QString       name            = folder_list.at(i).absoluteFilePath();
        QFileInfoList child_file_list = GetFileList(name);
        file_list.append(child_file_list);
    }

    return file_list;
}

int WriteFileDataEnd(QString path, QString str)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        return -1;
    }
    QByteArray bytes;
    for (int i = 0; i < str.count(); i++)
    {
        bytes.append(str.at(i));
    }
    bytes.append('\n');
    file.write(bytes, bytes.length());
    file.close();
    return 0;
}

bool PathIsExist(QString path)
{
    QFileInfo fileInfo(path);
    QString   folderPath = fileInfo.path();
    QDir      dir(folderPath);
    if (!dir.exists())
    {
        dir.mkpath(folderPath);
        return false;
    }
    return fileInfo.exists();
}

bool CsvAppend(QString path, QStringList data)
{
    QString result = data.join(',');
    WriteFileDataEnd(path, result);
    return true;
}

QStringList QList2QStringList(QList<QString> data)
{
    QStringList list;
    for (QString s : data)
    {
        list.append(s);
    }
    return list;
}

bool CsvAppend(QString path, QList<QString> header, QList<QString> data)
{
    if (!PathIsExist(path))
    {
        CsvAppend(path, QList2QStringList(header));
    }
    return CsvAppend(path, QList2QStringList(data));
}
