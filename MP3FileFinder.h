// MP3FileFinder.h
#ifndef MP3FILEFINDER_H
#define MP3FILEFINDER_H

#include <QString>
#include <QDir>

class MP3FileFinder {
public:
    MP3FileFinder(const QString& musicPath = "D:/music");

    // Получить список MP3 файлов
    QList<QFileInfo> getMP3Files() const;

    // Найти файл по ID и данным трека
    QString findFileByTrack(int id, const QString& title, const QString& artist,
                            const QString& album, int year, const QString& genre, int duration) const;

    QString getMusicPath() const { return musicPath; }

private:
    QString musicPath;
};

#endif // MP3FILEFINDER_H


