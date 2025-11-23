// CSVReader.cpp
#include "CSVReader.h"
#include "MusicCatalog.h"
#include "CSVParser.h"
#include <QFile>
#include <QTextStream>

bool CSVReader::loadFromCSV(MusicCatalog& catalog, const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // Пропускаем заголовок
    QString header = stream.readLine();
    if (header != "id,title,artist,album,year,genre,duration,filepath" &&
        header != "id,title,artist,album,year,genre,duration") {
        // Поддерживаем старый формат без filepath
        if (header == "id,title,artist,album,year,genre,duration") {
            // Продолжаем загрузку, но без filepath
        } else {
            file.close();
            return false;
        }
    }

    bool hasFilePath = header.contains("filepath");

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.isEmpty()) continue;

        QStringList fields = CSVParser::parseLine(line);

        if (fields.size() >= 7) {
            // Убираем кавычки если есть
            QString title = CSVParser::unescapeField(fields[1]);
            QString artist = CSVParser::unescapeField(fields[2]);
            QString album = CSVParser::unescapeField(fields[3]);
            QString genre = CSVParser::unescapeField(fields[5]);
            QString filePath = fields.size() >= 8 ? CSVParser::unescapeField(fields[7]) : "";

            int id = fields[0].toInt();
            int year = fields[4].toInt();
            int duration = fields[6].toInt();

            // Добавляем трек в каталог с сохранением исходного ID
            if (filePath.isEmpty()) {
                catalog.addTrackWithId(id, title, artist, album, year, genre, duration, "");
            } else {
                catalog.addTrackWithId(id, title, artist, album, year, genre, duration, filePath);
            }
        }
    }

    file.close();
    return true;
}


