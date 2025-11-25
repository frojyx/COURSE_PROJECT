// MP3FileNameParser.cpp
#include "MP3FileNameParser.h"
#include <QFileInfo>

namespace {
    bool parseFullFormat(const QStringList& parts, QString& title, QString& artist,
                        QString* album, int* year, QString* genre, int* duration) {
        if (parts.size() < 7) {
            return false;
        }
        
        title = parts[1];
        artist = parts[2];
        
        if (album != nullptr) {
            *album = parts[3];
        }
        if (year != nullptr) {
            bool ok;
            int yearValue = parts[4].toInt(&ok);
            if (ok) {
                *year = yearValue;
            }
        }
        if (genre != nullptr) {
            *genre = parts[5];
        }
        if (duration != nullptr) {
            bool ok;
            int durValue = parts[6].toInt(&ok);
            if (ok) {
                *duration = durValue;
            }
        }
        return !title.isEmpty() && !artist.isEmpty();
    }
    
    bool parseMinimalFormat(const QStringList& parts, QString& title, QString& artist) {
        if (parts.size() < 3) {
            return false;
        }
        title = parts[1];
        artist = parts[2];
        return !title.isEmpty() && !artist.isEmpty();
    }
    
    bool parseIdFormat(const QString& baseName, QString& title, QString& artist,
                      QString* album, int* year, QString* genre, int* duration) {
        QStringList parts = baseName.split('.');
        
        if (parts.size() >= 7) {
            return parseFullFormat(parts, title, artist, album, year, genre, duration);
        }
        if (parts.size() >= 3) {
            return parseMinimalFormat(parts, title, artist);
        }
        return false;
    }
    
    int findDotIndex(const QString& baseName) {
        int dotIndex = baseName.lastIndexOf('.');
        if (dotIndex == -1 || dotIndex == 0) {
            dotIndex = baseName.indexOf('.');
        }
        return dotIndex;
    }
}

bool MP3FileNameParser::parseFileName(const QString& fileName, QString& title, QString& artist,
                                     QString* album, int* year, QString* genre, int* duration) {
    // Формат: названиеПесни.исполнитель.mp3
    // или полный формат: ID.название.исполнитель.альбом.год.жанр.длительность.mp3
    QString baseName = QFileInfo(fileName).baseName();

    // Если имя начинается с цифры (ID), это уже обработанный файл
    if (!baseName.isEmpty() && baseName[0].isDigit()) {
        return parseIdFormat(baseName, title, artist, album, year, genre, duration);
    }

    // Для нового формата: названиеПесни.исполнитель
    int dotIndex = findDotIndex(baseName);
    if (dotIndex == -1) {
        return false;
    }

    title = baseName.left(dotIndex);
    artist = baseName.mid(dotIndex + 1);

    return !title.isEmpty() && !artist.isEmpty();
}




