// MP3FileOperations.h
#ifndef MP3FILEOPERATIONS_H
#define MP3FILEOPERATIONS_H

#include <QString>

class MP3FileOperations {
public:
    // Создать новое имя файла: ID.название.исполнитель.альбом.год.жанр.длительность.mp3
    static QString createNewFileName(int id, const QString& title, const QString& artist,
                                     const QString& album, int year, const QString& genre, int duration);

    // Переименовать файл
    static bool renameFile(const QString& oldPath, const QString& newFileName);

    // Открыть MP3 файл
    static bool openMP3File(const QString& filePath);
};

#endif // MP3FILEOPERATIONS_H


