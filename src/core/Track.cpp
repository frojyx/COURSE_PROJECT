// Track.cpp
#include "core/Track.h"
#include <QString>
#include <QTextStream>
#include <algorithm>
#include "file_operations/TXTParser.h"

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

bool Track::isFromYandexMusic() const {
    return !filePath.isEmpty() && 
           (filePath.startsWith("https://music.yandex.ru/") ||
            filePath.startsWith("https://music.yandex.ru/"));
}

// Перегрузка операторов сравнения
bool Track::operator==(const Track& other) const { // NOSONAR: Проект использует C++17, = default требует C++20
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

// Операторы << и >> теперь определены как hidden friends в Track.h

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
