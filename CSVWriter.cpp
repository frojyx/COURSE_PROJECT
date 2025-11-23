// CSVWriter.cpp
#include "CSVWriter.h"
#include "MusicCatalog.h"
#include "CSVParser.h"
#include <QFile>
#include <QTextStream>

bool CSVWriter::saveToCSV(const MusicCatalog& catalog, const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // Заголовок CSV
    stream << "id,title,artist,album,year,genre,duration,filepath\n";

    QList<Track> tracks = catalog.findAllTracks();
    for (const Track& track : tracks) {
        // Экранируем поля, которые могут содержать запятые
        QString title = CSVParser::escapeField(track.getTitle());
        QString artist = CSVParser::escapeField(track.getArtist());
        QString album = CSVParser::escapeField(track.getAlbum());
        QString genre = CSVParser::escapeField(track.getGenre());
        QString filePath = CSVParser::escapeField(track.getFilePath());

        stream << track.getId() << ","
               << title << ","
               << artist << ","
               << album << ","
               << track.getYear() << ","
               << genre << ","
               << track.getDuration() << ","
               << filePath << "\n";
    }

    file.close();
    return true;
}


