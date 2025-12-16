// MP3FileOperations.cpp
#include "mp3/MP3FileOperations.h"
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QDesktopServices>

QString MP3FileOperations::createNewFileName(int id, const QString& title, const QString& artist,
                                             const QString& album, int year, const QString& genre, int duration) {
    // Формат: ID.название.исполнитель.альбом.год.жанр.длительность.mp3
    QString newFileName = QString("%1.%2.%3.%4.%5.%6.%7.mp3")
                              .arg(id)
                              .arg(title)
                              .arg(artist)
                              .arg(album)
                              .arg(year)
                              .arg(genre)
                              .arg(duration);

    // Заменяем недопустимые символы для имени файла
    newFileName.replace('/', '_');
    newFileName.replace('\\', '_');
    newFileName.replace(':', '_');
    newFileName.replace('*', '_');
    newFileName.replace('?', '_');
    newFileName.replace('"', '_');
    newFileName.replace('<', '_');
    newFileName.replace('>', '_');
    newFileName.replace('|', '_');

    return newFileName;
}

bool MP3FileOperations::renameFile(const QString& oldPath, const QString& newFileName) {
    QFileInfo fileInfo(oldPath);
    QString newPath = fileInfo.absolutePath() + "/" + newFileName;

    QFile file(oldPath);
    if (QFile::exists(newPath)) {
        // Если файл с таким именем уже существует, удаляем его
        QFile::remove(newPath);
    }

    return file.rename(newPath);
}

bool MP3FileOperations::openMP3File(const QString& filePath) {
    // Используем QDesktopServices для открытия файла системным приложением
    QUrl url = QUrl::fromLocalFile(filePath);
    return QDesktopServices::openUrl(url);
}




