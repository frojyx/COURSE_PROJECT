// MP3FileFinder.cpp
#include "MP3FileFinder.h"
#include "MP3FileOperations.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>

MP3FileFinder::MP3FileFinder(const QString& musicPath)
    : musicPath(musicPath)
{
}

QList<QFileInfo> MP3FileFinder::getMP3Files() const {
    QList<QFileInfo> files;
    QDir dir(musicPath);

    if (!dir.exists()) {
        return files;
    }

    QStringList filters;
    filters << "*.mp3" << "*.MP3";

    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
    for (const QFileInfo& fileInfo : fileList) {
        // Проверяем, что файл не начинается с цифры (ID) - это уже обработанные файлы
        QString baseName = fileInfo.baseName();
        if (!baseName.isEmpty() && baseName[0].isDigit() && baseName.contains('.')) {
            // Файл уже обработан, пропускаем
            continue;
        }
        files.append(fileInfo);
    }

    return files;
}

QString MP3FileFinder::findFileByTrack(int id, const QString& title, const QString& artist,
                                       const QString& album, int year, const QString& genre, int duration) const {
    QDir dir(musicPath);

    if (!dir.exists()) {
        return QString();
    }

    // Сначала ищем файл с новым форматом имени
    QString expectedFileName = MP3FileOperations::createNewFileName(id, title, artist, album, year, genre, duration);
    QString expectedPath = dir.absoluteFilePath(expectedFileName);

    if (QFile::exists(expectedPath)) {
        return expectedPath;
    }

    // Если не найден, ищем по старому формату
    QString oldFormat = QString("%1.%2.mp3").arg(title).arg(artist);
    QString oldPath = dir.absoluteFilePath(oldFormat);

    if (QFile::exists(oldPath)) {
        return oldPath;
    }

    // Ищем любой файл, начинающийся с ID
    QStringList filters;
    filters << QString("%1.*.mp3").arg(id);

    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
    if (!fileList.isEmpty()) {
        return fileList.first().absoluteFilePath();
    }

    return QString();
}




