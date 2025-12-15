// TrackSearcher.cpp
#include "TrackSearcher.h"
#include "TrackRepository.h"
#include "TrackSearchParams.h"

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

namespace {
    bool matchesFilters(const Track& track, const TrackSearchParams& params) {
        // Фильтр по названию
        if (!params.title.isEmpty() && !track.getTitle().contains(params.title, Qt::CaseInsensitive)) {
            return false;
        }

        // Фильтр по исполнителю
        if (!params.artist.isEmpty() && !track.getArtist().contains(params.artist, Qt::CaseInsensitive)) {
            return false;
        }

        // Фильтр по альбому
        if (!params.album.isEmpty() && !track.getAlbum().contains(params.album, Qt::CaseInsensitive)) {
            return false;
        }

        // Фильтр по жанру
        if (!params.genre.isEmpty() && !track.getGenre().contains(params.genre, Qt::CaseInsensitive)) {
            return false;
        }

        // Фильтр по году
        if (params.minYear >= 1900 && track.getYear() < params.minYear) {
            return false;
        }
        if (params.maxYear <= 2100 && params.maxYear >= 1900 && track.getYear() > params.maxYear) {
            return false;
        }

        // Фильтр по длительности
        if (params.minDuration > 1 && track.getDuration() < params.minDuration) {
            return false;
        }
        if (params.maxDuration < 3600 && track.getDuration() > params.maxDuration) {
            return false;
        }

        return true;
    }
}

QList<Track> TrackSearcher::searchTracksWithFilters(const TrackSearchParams& params) const {
    QList<Track> result;
    QList<Track> tracks = repository.findAllTracks();

    for (const Track& track : tracks) {
        if (matchesFilters(track, params)) {
            result.append(track);
        }
    }

    return result;
}




