// MusicCatalog.cpp
#include "MusicCatalog.h"

MusicCatalog::MusicCatalog()
    : searcher(repository), sorter(repository)
{
}

void MusicCatalog::addTrack(const QString& title, const QString& artist,
                            const QString& album, int year, const QString& genre, int duration) {
    repository.addTrack(title, artist, album, year, genre, duration);
}

void MusicCatalog::addTrack(const QString& title, const QString& artist,
                            const QString& album, int year, const QString& genre, int duration, const QString& filePath) {
    repository.addTrack(title, artist, album, year, genre, duration, filePath);
}

void MusicCatalog::addTrackWithId(int id, const QString& title, const QString& artist,
                                  const QString& album, int year, const QString& genre, int duration, const QString& filePath) {
    repository.addTrackWithId(id, title, artist, album, year, genre, duration, filePath);
}

bool MusicCatalog::removeTrack(int id) {
    return repository.removeTrack(id);
}

bool MusicCatalog::updateTrack(int id, const Track& updatedTrack) {
    return repository.updateTrack(id, updatedTrack);
}

Track* MusicCatalog::findTrackById(int id) {
    return repository.findTrackById(id);
}

const Track* MusicCatalog::findTrackById(int id) const {
    return repository.findTrackById(id);
}

QList<Track> MusicCatalog::findAllTracks() const {
    return repository.findAllTracks();
}

QList<Track> MusicCatalog::findTracksByTitle(const QString& title) const {
    return searcher.findTracksByTitle(title);
}

QList<Track> MusicCatalog::findTracksByArtist(const QString& artist) const {
    return searcher.findTracksByArtist(artist);
}

QList<Track> MusicCatalog::findTracksByAlbum(const QString& album) const {
    return searcher.findTracksByAlbum(album);
}

QList<Track> MusicCatalog::findTracksByGenre(const QString& genre) const {
    return searcher.findTracksByGenre(genre);
}

QList<Track> MusicCatalog::findTracksByYearRange(int startYear, int endYear) const {
    return searcher.findTracksByYearRange(startYear, endYear);
}

QList<Track> MusicCatalog::searchTracks(const QString& searchTerm) const {
    return searcher.searchTracks(searchTerm);
}

QList<Track> MusicCatalog::searchTracksWithFilters(const QString& title, const QString& artist,
                                                   const QString& album, const QString& genre,
                                                   int minYear, int maxYear,
                                                   int minDuration, int maxDuration) const {
    return searcher.searchTracksWithFilters(title, artist, album, genre, minYear, maxYear, minDuration, maxDuration);
}

void MusicCatalog::sortByTitle(bool ascending) {
    sorter.sortByTitle(ascending);
}

void MusicCatalog::sortByArtist(bool ascending) {
    sorter.sortByArtist(ascending);
}

void MusicCatalog::sortByYear(bool ascending) {
    sorter.sortByYear(ascending);
}

void MusicCatalog::sortByDuration(bool ascending) {
    sorter.sortByDuration(ascending);
}

int MusicCatalog::getTrackCount() const {
    return repository.getTrackCount();
}

int MusicCatalog::getNextId() const {
    return repository.getNextId();
}

void MusicCatalog::updateNextId() {
    repository.updateNextId();
}
