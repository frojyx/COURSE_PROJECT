// TXTWriter.cpp
#include "TXTWriter.h"
#include "MusicCatalog.h"
#include "TXTParser.h"
#include "FileException.h"
#include <QFile>
#include <QTextStream>

bool TXTWriter::saveToTXT(const MusicCatalog& catalog, const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw FileException(filename, "записи");
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // Заголовок TXT
    stream << "id" << TXTParser::FIELD_SEPARATOR
           << "title" << TXTParser::FIELD_SEPARATOR
           << "artist" << TXTParser::FIELD_SEPARATOR
           << "album" << TXTParser::FIELD_SEPARATOR
           << "year" << TXTParser::FIELD_SEPARATOR
           << "genre" << TXTParser::FIELD_SEPARATOR
           << "duration" << TXTParser::FIELD_SEPARATOR
           << "filepath\n";

    QList<Track> tracks = catalog.findAllTracks();
    for (const Track& track : tracks) {
        // Экранируем поля
        QString title = TXTParser::escapeField(track.getTitle());
        QString artist = TXTParser::escapeField(track.getArtist());
        QString album = TXTParser::escapeField(track.getAlbum());
        QString genre = TXTParser::escapeField(track.getGenre());
        QString filePath = TXTParser::escapeField(track.getFilePath());

        stream << track.getId() << TXTParser::FIELD_SEPARATOR
               << title << TXTParser::FIELD_SEPARATOR
               << artist << TXTParser::FIELD_SEPARATOR
               << album << TXTParser::FIELD_SEPARATOR
               << track.getYear() << TXTParser::FIELD_SEPARATOR
               << genre << TXTParser::FIELD_SEPARATOR
               << track.getDuration() << TXTParser::FIELD_SEPARATOR
               << filePath << "\n";
    }

    file.close();
    return true;
}

