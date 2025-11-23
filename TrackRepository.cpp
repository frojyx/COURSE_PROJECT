// TrackRepository.cpp
#include "TrackRepository.h"

TrackRepository::TrackRepository() : nextId(1) {}

void TrackRepository::addTrack(const Track& track) {
    Track newTrack = track;
    newTrack.setId(nextId++);
    tracks.append(newTrack);
}

void TrackRepository::addTrack(const QString& title, const QString& artist,
                                const QString& album, int year, const QString& genre, int duration) {
    Track track(nextId++, title, artist, album, year, genre, duration);
    tracks.append(track);
}

void TrackRepository::addTrack(const QString& title, const QString& artist,
                                const QString& album, int year, const QString& genre, int duration, const QString& filePath) {
    Track track(nextId++, title, artist, album, year, genre, duration, filePath);
    tracks.append(track);
}

void TrackRepository::addTrackWithId(int id, const QString& title, const QString& artist,
                                      const QString& album, int year, const QString& genre, int duration, const QString& filePath) {
    Track track(id, title, artist, album, year, genre, duration, filePath);
    tracks.append(track);
    if (id >= nextId) {
        nextId = id + 1;
    }
}

bool TrackRepository::removeTrack(int id) {
    for (int i = 0; i < tracks.size(); ++i) {
        if (tracks[i].getId() == id) {
            tracks.removeAt(i);
            return true;
        }
    }
    return false;
}

bool TrackRepository::updateTrack(int id, const Track& updatedTrack) {
    for (int i = 0; i < tracks.size(); ++i) {
        if (tracks[i].getId() == id) {
            tracks[i] = updatedTrack;
            tracks[i].setId(id); // Сохраняем оригинальный ID
            return true;
        }
    }
    return false;
}

Track* TrackRepository::findTrackById(int id) {
    for (int i = 0; i < tracks.size(); ++i) {
        if (tracks[i].getId() == id) {
            return &tracks[i];
        }
    }
    return nullptr;
}

const Track* TrackRepository::findTrackById(int id) const {
    for (int i = 0; i < tracks.size(); ++i) {
        if (tracks[i].getId() == id) {
            return &tracks[i];
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

