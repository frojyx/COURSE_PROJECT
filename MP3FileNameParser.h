// MP3FileNameParser.h
#ifndef MP3FILENAMEPARSER_H
#define MP3FILENAMEPARSER_H

#include <QString>
#include <QFileInfo>

class MP3FileNameParser {
public:
    // Парсинг имени файла: названиеПесни.исполнитель.mp3
    // или полный формат: ID.название.исполнитель.альбом.год.жанр.длительность.mp3
    // Параметры album, year, genre, duration опциональны (передавайте nullptr/пустые строки если не нужны)
    static bool parseFileName(const QString& fileName, QString& title, QString& artist,
                              QString* album = nullptr, int* year = nullptr,
                              QString* genre = nullptr, int* duration = nullptr);
};

#endif // MP3FILENAMEPARSER_H





