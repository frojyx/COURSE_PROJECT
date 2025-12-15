// TXTWriter.cpp
#include "TXTWriter.h"
#include "MusicCatalog.h"
#include "Track.h"
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
        // Используем перегруженный оператор << для вывода трека
        stream << track << "\n";
    }

    file.close();
    return true;
}

