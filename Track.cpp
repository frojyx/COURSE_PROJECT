// Track.cpp
#include "Track.h"
#include <QString>
#include <QTextStream>
#include <algorithm>
#include "TXTParser.h"

Track::Track() : id(0), year(0), duration(0) {}

Track::Track(int id, const QString& title, const QString& artist,
             const QString& album, int year, const QString& genre, int duration)
    : id(id), title(title), artist(artist), album(album),
    year(year), genre(genre), duration(duration), filePath("") {}

Track::Track(int id, const TrackParams& params)
    : id(id), title(params.title), artist(params.artist), album(params.album),
    year(params.year), genre(params.genre), duration(params.duration), filePath(params.filePath) {}

QString Track::getFormattedDuration() const {
    int minutes = duration / 60;
    int seconds = duration % 60;
    return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
}

bool Track::matchesSearch(const QString& searchTerm) const {
    if (searchTerm.isEmpty()) return true;

    QString lowerSearch = searchTerm.toLower();

    return title.toLower().contains(lowerSearch) ||
           artist.toLower().contains(lowerSearch) ||
           album.toLower().contains(lowerSearch) ||
           genre.toLower().contains(lowerSearch);
}

// Перегрузка операторов сравнения
bool Track::operator==(const Track& other) const {
    return id == other.id &&
           title == other.title &&
           artist == other.artist &&
           album == other.album &&
           year == other.year &&
           genre == other.genre &&
           duration == other.duration &&
           filePath == other.filePath;
}

bool Track::operator!=(const Track& other) const {
    return !(*this == other);
}

bool Track::operator<(const Track& other) const {
    // Сравнение по ID (для сортировки по умолчанию)
    if (id != other.id) {
        return id < other.id;
    }
    // Если ID одинаковые, сравниваем по названию
    if (title != other.title) {
        return title < other.title;
    }
    // Если названия одинаковые, сравниваем по исполнителю
    return artist < other.artist;
}

bool Track::operator>(const Track& other) const {
    return other < *this;
}

bool Track::operator<=(const Track& other) const {
    return !(other < *this);
}

bool Track::operator>=(const Track& other) const {
    return !(*this < other);
}

// Дружественная функция для вывода трека в поток
QTextStream& operator<<(QTextStream& stream, const Track& track) {
    // Экранируем поля
    QString title = TXTParser::escapeField(track.title);
    QString artist = TXTParser::escapeField(track.artist);
    QString album = TXTParser::escapeField(track.album);
    QString genre = TXTParser::escapeField(track.genre);
    QString filePath = TXTParser::escapeField(track.filePath);

    stream << track.id << TXTParser::FIELD_SEPARATOR
           << title << TXTParser::FIELD_SEPARATOR
           << artist << TXTParser::FIELD_SEPARATOR
           << album << TXTParser::FIELD_SEPARATOR
           << track.year << TXTParser::FIELD_SEPARATOR
           << genre << TXTParser::FIELD_SEPARATOR
           << track.duration << TXTParser::FIELD_SEPARATOR
           << filePath;
    return stream;
}

// Дружественная функция для чтения трека из потока
QTextStream& operator>>(QTextStream& stream, Track& track) {
    QString line = stream.readLine();
    if (line.isEmpty()) {
        return stream;
    }

    QStringList fields = TXTParser::parseLine(line);
    if (fields.size() < 7) {
        return stream;
    }

    bool ok;
    track.id = fields[0].toInt(&ok);
    if (!ok) {
        track.id = 0;
    }

    track.title = TXTParser::unescapeField(fields[1]);
    track.artist = TXTParser::unescapeField(fields[2]);
    track.album = TXTParser::unescapeField(fields[3]);
    track.year = fields[4].toInt(&ok);
    if (!ok) {
        track.year = 0;
    }
    track.genre = TXTParser::unescapeField(fields[5]);
    track.duration = fields[6].toInt(&ok);
    if (!ok) {
        track.duration = 0;
    }
    track.filePath = fields.size() >= 8 ? TXTParser::unescapeField(fields[7]) : "";

    return stream;
}

// Дружественные функции для сравнения треков по различным критериям
bool compareTracksByTitle(const Track& a, const Track& b) {
    if (a.title != b.title) {
        return a.title < b.title;
    }
    return a.artist < b.artist;
}

bool compareTracksByArtist(const Track& a, const Track& b) {
    if (a.artist != b.artist) {
        return a.artist < b.artist;
    }
    return a.title < b.title;
}

bool compareTracksByYear(const Track& a, const Track& b) {
    if (a.year != b.year) {
        return a.year < b.year;
    }
    return a.title < b.title;
}

bool compareTracksByDuration(const Track& a, const Track& b) {
    if (a.duration != b.duration) {
        return a.duration < b.duration;
    }
    return a.title < b.title;
}
