// TrackRepository.h
#ifndef TRACKREPOSITORY_H
#define TRACKREPOSITORY_H

#include "core/Track.h"
#include <QList>

class TrackRepository {
public:
    TrackRepository();

    // CRUD операции
    void addTrack(const Track& track);
    void addTrack(const QString& title, const QString& artist,
                  const QString& album, int year, const QString& genre, int duration);
    void addTrack(const QString& title, const QString& artist,
                  const QString& album, int year, const QString& genre, int duration, const QString& filePath);
    void addTrackWithId(int id, const TrackParams& params);
    bool removeTrack(int id);
    bool updateTrack(int id, const Track& updatedTrack);

    // Получение треков
    Track* findTrackById(int id);
    const Track* findTrackById(int id) const;
    QList<Track> findAllTracks() const { return tracks; }

    // Статистика
    int getTrackCount() const { return tracks.size(); }
    int getNextId() const { return nextId; }
    void updateNextId();

    // Для сортировки
    void setTracks(const QList<Track>& newTracks);

private:
    QList<Track> tracks;
    int nextId = 1;
};

#endif // TRACKREPOSITORY_H

