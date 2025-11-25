// MusicCatalog.h
#ifndef MUSICCATALOG_H
#define MUSICCATALOG_H

#include "Track.h"
#include "TrackRepository.h"
#include "TrackSearcher.h"
#include "TrackSorter.h"
#include "TrackSearchParams.h"
#include <QList>
#include <QString>

class MusicCatalog {
public:
    MusicCatalog();

    // Основные операции
    void addTrack(const QString& title, const QString& artist,
                  const QString& album, int year, const QString& genre, int duration);
    void addTrack(const QString& title, const QString& artist,
                  const QString& album, int year, const QString& genre, int duration, const QString& filePath);
    void addTrackWithId(int id, const TrackAddParams& params);
    bool removeTrack(int id);
    bool updateTrack(int id, const Track& updatedTrack);

    // Поиск
    Track* findTrackById(int id);
    const Track* findTrackById(int id) const;
    QList<Track> findAllTracks() const;
    QList<Track> findTracksByTitle(const QString& title) const;
    QList<Track> findTracksByArtist(const QString& artist) const;
    QList<Track> findTracksByAlbum(const QString& album) const;
    QList<Track> findTracksByGenre(const QString& genre) const;
    QList<Track> findTracksByYearRange(int startYear, int endYear) const;
    QList<Track> searchTracks(const QString& searchTerm) const;
    QList<Track> searchTracksWithFilters(const TrackSearchParams& params) const;

    // Сортировка
    void sortByTitle(bool ascending = true);
    void sortByArtist(bool ascending = true);
    void sortByYear(bool ascending = true);
    void sortByDuration(bool ascending = true);

    // Статистика
    int getTrackCount() const;
    int getNextId() const;
    void updateNextId();

private:
    TrackRepository repository;
    TrackSearcher searcher;
    TrackSorter sorter;
};

#endif // MUSICCATALOG_H
