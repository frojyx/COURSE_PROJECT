// MP3FileNameParser.cpp
#include "MP3FileNameParser.h"
#include <QFileInfo>

bool MP3FileNameParser::parseFileName(const QString& fileName, QString& title, QString& artist,
                                     QString* album, int* year, QString* genre, int* duration) {
    // Формат: названиеПесни.исполнитель.mp3
    // или полный формат: ID.название.исполнитель.альбом.год.жанр.длительность.mp3
    QString baseName = QFileInfo(fileName).baseName();

    // Если имя начинается с цифры (ID), это уже обработанный файл
    if (!baseName.isEmpty() && baseName[0].isDigit()) {
        // Пытаемся парсить формат ID.название.исполнитель.альбом.год.жанр.длительность
        QStringList parts = baseName.split('.');
        if (parts.size() >= 7) {
            // Пропускаем ID (первый элемент)
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
        } else if (parts.size() >= 3) {
            // Минимальный формат: ID.название.исполнитель
            title = parts[1];
            artist = parts[2];
            return !title.isEmpty() && !artist.isEmpty();
        }
    }

    // Для нового формата: названиеПесни.исполнитель
    int dotIndex = baseName.lastIndexOf('.');
    if (dotIndex == -1 || dotIndex == 0) {
        // Если нет точки или точка в начале, пробуем разделить по первой точке
        dotIndex = baseName.indexOf('.');
        if (dotIndex == -1) {
            return false;
        }
    }

    title = baseName.left(dotIndex);
    artist = baseName.mid(dotIndex + 1);

    return !title.isEmpty() && !artist.isEmpty();
}


