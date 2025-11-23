// TrackSearcher.cpp
#include "TrackSearcher.h"
#include "TrackRepository.h"

TrackSearcher::TrackSearcher(const TrackRepository& repository)
    : repository(repository)
{
}

QList<Track> TrackSearcher::findTracksByTitle(const QString& title) const {
    QList<Track> result;
    QList<Track> tracks = repository.findAllTracks();
    for (const Track& track : tracks) {
        if (track.getTitle().contains(title, Qt::CaseInsensitive)) {
            result.append(track);
        }
    }
    return result;
}

QList<Track> TrackSearcher::findTracksByArtist(const QString& artist) const {
    QList<Track> result;
    QList<Track> tracks = repository.findAllTracks();
    for (const Track& track : tracks) {
        if (track.getArtist().contains(artist, Qt::CaseInsensitive)) {
            result.append(track);
        }
    }
    return result;
}

QList<Track> TrackSearcher::findTracksByAlbum(const QString& album) const {
    QList<Track> result;
    QList<Track> tracks = repository.findAllTracks();
    for (const Track& track : tracks) {
        if (track.getAlbum().contains(album, Qt::CaseInsensitive)) {
            result.append(track);
        }
    }
    return result;
}

QList<Track> TrackSearcher::findTracksByGenre(const QString& genre) const {
    QList<Track> result;
    QList<Track> tracks = repository.findAllTracks();
    for (const Track& track : tracks) {
        if (track.getGenre().contains(genre, Qt::CaseInsensitive)) {
            result.append(track);
        }
    }
    return result;
}

QList<Track> TrackSearcher::findTracksByYearRange(int startYear, int endYear) const {
    QList<Track> result;
    QList<Track> tracks = repository.findAllTracks();
    for (const Track& track : tracks) {
        if (track.getYear() >= startYear && track.getYear() <= endYear) {
            result.append(track);
        }
    }
    return result;
}

QList<Track> TrackSearcher::searchTracks(const QString& searchTerm) const {
    QList<Track> result;
    QList<Track> tracks = repository.findAllTracks();
    for (const Track& track : tracks) {
        if (track.matchesSearch(searchTerm)) {
            result.append(track);
        }
    }
    return result;
}

QList<Track> TrackSearcher::searchTracksWithFilters(const QString& title, const QString& artist,
                                                     const QString& album, const QString& genre,
                                                     int minYear, int maxYear,
                                                     int minDuration, int maxDuration) const {
    QList<Track> result;
    QList<Track> tracks = repository.findAllTracks();

    for (const Track& track : tracks) {
        bool matches = true;

        // Фильтр по названию
        if (!title.isEmpty() && !track.getTitle().contains(title, Qt::CaseInsensitive)) {
            matches = false;
        }

        // Фильтр по исполнителю
        if (matches && !artist.isEmpty() && !track.getArtist().contains(artist, Qt::CaseInsensitive)) {
            matches = false;
        }

        // Фильтр по альбому
        if (matches && !album.isEmpty() && !track.getAlbum().contains(album, Qt::CaseInsensitive)) {
            matches = false;
        }

        // Фильтр по жанру
        if (matches && !genre.isEmpty() && !track.getGenre().contains(genre, Qt::CaseInsensitive)) {
            matches = false;
        }

        // Фильтр по году
        if (matches && minYear >= 1900 && track.getYear() < minYear) {
            matches = false;
        }
        if (matches && maxYear <= 2100 && maxYear >= 1900 && track.getYear() > maxYear) {
            matches = false;
        }

        // Фильтр по длительности
        if (matches && minDuration > 1 && track.getDuration() < minDuration) {
            matches = false;
        }
        if (matches && maxDuration < 3600 && track.getDuration() > maxDuration) {
            matches = false;
        }

        if (matches) {
            result.append(track);
        }
    }

    return result;
}


