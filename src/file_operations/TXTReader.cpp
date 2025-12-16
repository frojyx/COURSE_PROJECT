// TXTReader.cpp
#include "file_operations/TXTReader.h"
#include "core/MusicCatalog.h"
#include "file_operations/TXTParser.h"
#include "exceptions/FileException.h"
#include "exceptions/ParseException.h"
#include "exceptions/ValidationException.h"
#include "core/TrackSearchParams.h"
#include <QFile>
#include <QTextStream>

bool TXTReader::loadFromTXT(MusicCatalog& catalog, const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw FileException(filename, "открытии");
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // Пропускаем заголовок
    QString header = stream.readLine();
    QString expectedHeader = "id" + TXTParser::FIELD_SEPARATOR +
                            "title" + TXTParser::FIELD_SEPARATOR +
                            "artist" + TXTParser::FIELD_SEPARATOR +
                            "album" + TXTParser::FIELD_SEPARATOR +
                            "year" + TXTParser::FIELD_SEPARATOR +
                            "genre" + TXTParser::FIELD_SEPARATOR +
                            "duration" + TXTParser::FIELD_SEPARATOR +
                            "filepath";
    
    if (QString oldHeader = "id" + TXTParser::FIELD_SEPARATOR +
                           "title" + TXTParser::FIELD_SEPARATOR +
                           "artist" + TXTParser::FIELD_SEPARATOR +
                           "album" + TXTParser::FIELD_SEPARATOR +
                           "year" + TXTParser::FIELD_SEPARATOR +
                           "genre" + TXTParser::FIELD_SEPARATOR +
                           "duration";
        header != expectedHeader && header != oldHeader) {
        file.close();
        throw ParseException(1, QString("Неверный заголовок файла. Ожидался формат: id|||title|||artist|||..."));
    }

    int lineNumber = 1; // Учитываем заголовок
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        lineNumber++;
        if (line.isEmpty()) continue;

        QStringList fields = TXTParser::parseLine(line);

        if (fields.size() < 7) {
            throw ParseException(lineNumber, 
                                QString("Недостаточно полей в строке. Ожидалось минимум 7, получено %1").arg(fields.size()));
        }

        // Убираем экранирование если есть
        QString title = TXTParser::unescapeField(fields[1]);
        QString artist = TXTParser::unescapeField(fields[2]);
        QString album = TXTParser::unescapeField(fields[3]);
        QString genre = TXTParser::unescapeField(fields[5]);
        QString filePath = fields.size() >= 8 ? TXTParser::unescapeField(fields[7]) : "";

        bool ok;
        int id = fields[0].toInt(&ok);
        if (!ok) {
            throw ParseException(lineNumber, 
                                QString("Неверный формат ID: '%1'").arg(fields[0]));
        }

        int year = fields[4].toInt(&ok);
        if (!ok) {
            throw ParseException(lineNumber, 
                                QString("Неверный формат года: '%1'").arg(fields[4]));
        }

        int duration = fields[6].toInt(&ok);
        if (!ok) {
            throw ParseException(lineNumber, 
                                QString("Неверный формат длительности: '%1'").arg(fields[6]));
        }

        // Валидация данных
        if (title.isEmpty()) {
            throw ValidationException("title", "название трека не может быть пустым");
        }
        if (artist.isEmpty()) {
            throw ValidationException("artist", "исполнитель не может быть пустым");
        }

            // Добавляем трек в каталог с сохранением исходного ID
        TrackAddParams params;
        params.title = title;
        params.artist = artist;
        params.album = album;
        params.year = year;
        params.genre = genre;
        params.duration = duration;
        params.filePath = filePath.isEmpty() ? "" : filePath;
        catalog.addTrackWithId(id, params);
    }

    file.close();
    return true;
}

