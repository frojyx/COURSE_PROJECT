// MP3FileManager.h
#ifndef MP3FILEMANAGER_H
#define MP3FILEMANAGER_H

#include "MP3FileFinder.h"
#include "MP3MetadataReader.h"
#include "MP3FileNameParser.h"
#include "MP3FileOperations.h"
#include <QString>
#include <QFileInfo>
#include <QList>

class MP3FileManager {
public:
    MP3FileManager(const QString& musicPath = "D:/music");

    // Получить список MP3 файлов
    QList<QFileInfo> getMP3Files() const;

    // Парсинг имени файла: названиеПесни.исполнитель.mp3
    // или полный формат: ID.название.исполнитель.альбом.год.жанр.длительность.mp3
    // Параметры album, year, genre, duration опциональны (передавайте nullptr/пустые строки если не нужны)
    static bool parseFileName(const QString& fileName, QString& title, QString& artist,
                              QString* album = nullptr, int* year = nullptr,
                              QString* genre = nullptr, int* duration = nullptr);

    // Чтение метаданных из MP3 файла (ID3 теги)
    // Параметры album, year, genre, duration опциональны (передавайте nullptr если не нужны)
    static bool readMP3Metadata(const QString& filePath, QString& title, QString& artist,
                                QString* album = nullptr, int* year = nullptr,
                                QString* genre = nullptr, int* duration = nullptr);

    // Создать новое имя файла: ID.название.исполнитель.альбом.год.жанр.длительность.mp3
    static QString createNewFileName(int id, const QString& title, const QString& artist,
                                     const QString& album, int year, const QString& genre, int duration);

    // Переименовать файл
    static bool renameFile(const QString& oldPath, const QString& newFileName);

    // Открыть MP3 файл
    static bool openMP3File(const QString& filePath);

    // Найти файл по ID и данным трека
    QString findFileByTrack(int id, const QString& title, const QString& artist,
                            const QString& album, int year, const QString& genre, int duration) const;

    QString getMusicPath() const;

private:
    MP3FileFinder fileFinder;
};

#endif // MP3FILEMANAGER_H
