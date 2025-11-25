// TrackRepository.cpp
#include "TrackRepository.h"
#include "TrackException.h"
#include "ValidationException.h"
#include "Track.h"

TrackRepository::TrackRepository() = default;

void TrackRepository::addTrack(const Track& track) {
    Track newTrack = track;
    int currentId = nextId;
    nextId = currentId + 1;
    newTrack.setId(currentId);
    tracks.append(newTrack);
}

void TrackRepository::addTrack(const QString& title, const QString& artist,
                                const QString& album, int year, const QString& genre, int duration) {
    if (title.isEmpty()) {
        throw ValidationException("title", "название трека не может быть пустым");
    }
    if (artist.isEmpty()) {
        throw ValidationException("artist", "исполнитель не может быть пустым");
    }
    Track track(nextId++, title, artist, album, year, genre, duration);
    tracks.append(track);
}

void TrackRepository::addTrack(const QString& title, const QString& artist,
                                const QString& album, int year, const QString& genre, int duration, const QString& filePath) {
    if (title.isEmpty()) {
        throw ValidationException("title", "название трека не может быть пустым");
    }
    if (artist.isEmpty()) {
        throw ValidationException("artist", "исполнитель не может быть пустым");
    }
    TrackParams params;
    params.title = title;
    params.artist = artist;
    params.album = album;
    params.year = year;
    params.genre = genre;
    params.duration = duration;
    params.filePath = filePath;
    int currentId = nextId;
    nextId = currentId + 1;
    Track track(currentId, params);
    tracks.append(track);
}

void TrackRepository::addTrackWithId(int id, const QString& title, const QString& artist,
                                      const QString& album, int year, const QString& genre, int duration, const QString& filePath) {
    TrackParams params;
    params.title = title;
    params.artist = artist;
    params.album = album;
    params.year = year;
    params.genre = genre;
    params.duration = duration;
    params.filePath = filePath;
    Track track(id, params);
    tracks.append(track);
    if (id >= nextId) {
        nextId = id + 1;
    }
}

bool TrackRepository::removeTrack(int id) {
    for (auto it = tracks.begin(); it != tracks.end(); ++it) {
        if (it->getId() == id) {
            tracks.erase(it);
            return true;
        }
    }
    // Возвращаем false, если трек не найден (для обратной совместимости)
    return false;
}

bool TrackRepository::updateTrack(int id, const Track& updatedTrack) {
    if (updatedTrack.getTitle().isEmpty()) {
        throw ValidationException("title", "название трека не может быть пустым");
    }
    if (updatedTrack.getArtist().isEmpty()) {
        throw ValidationException("artist", "исполнитель не может быть пустым");
    }
    
    for (Track& track : tracks) {
        if (track.getId() == id) {
            track = updatedTrack;
            track.setId(id); // Сохраняем оригинальный ID
            return true;
        }
    }
    // Возвращаем false, если трек не найден (для обратной совместимости)
    return false;
}

Track* TrackRepository::findTrackById(int id) {
    for (Track& track : tracks) {
        if (track.getId() == id) {
            return &track;
        }
    }
    return nullptr;
}

const Track* TrackRepository::findTrackById(int id) const {
    for (const Track& track : tracks) {
        if (track.getId() == id) {
            return &track;
        }
    }
    return nullptr;
}

void TrackRepository::updateNextId() {
    int maxId = 0;
    for (const Track& track : tracks) {
        if (track.getId() > maxId) {
            maxId = track.getId();
        }
    }
    nextId = maxId + 1;
}

void TrackRepository::setTracks(const QList<Track>& newTracks) {
    tracks = newTracks;
}

