// TrackSearcher.h
#ifndef TRACKSEARCHER_H
#define TRACKSEARCHER_H

#include "core/Track.h"
#include "core/TrackSearchParams.h"
#include <QList>
#include <QString>

class TrackRepository;

class TrackSearcher {
public:
    explicit TrackSearcher(const TrackRepository& repository);

    // Простой поиск
    QList<Track> searchTracks(const QString& searchTerm) const;

    // Поиск по полям
    QList<Track> findTracksByTitle(const QString& title) const;
    QList<Track> findTracksByArtist(const QString& artist) const;
    QList<Track> findTracksByAlbum(const QString& album) const;
    QList<Track> findTracksByGenre(const QString& genre) const;
    QList<Track> findTracksByYearRange(int startYear, int endYear) const;

    // Комбинированный поиск с фильтрами
    QList<Track> searchTracksWithFilters(const TrackSearchParams& params) const;

private:
    const TrackRepository& repository;
};

#endif // TRACKSEARCHER_H




