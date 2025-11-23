// Track.cpp
#include "Track.h"
#include <QString>
#include <algorithm>

Track::Track() : id(0), year(0), duration(0) {}

Track::Track(int id, const QString& title, const QString& artist,
             const QString& album, int year, const QString& genre, int duration)
    : id(id), title(title), artist(artist), album(album),
    year(year), genre(genre), duration(duration), filePath("") {}

Track::Track(int id, const QString& title, const QString& artist,
             const QString& album, int year, const QString& genre, int duration, const QString& filePath)
    : id(id), title(title), artist(artist), album(album),
    year(year), genre(genre), duration(duration), filePath(filePath) {}

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
