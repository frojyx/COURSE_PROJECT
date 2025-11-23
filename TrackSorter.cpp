// TrackSorter.cpp
#include "TrackSorter.h"
#include "TrackRepository.h"
#include "Track.h"
#include <algorithm>

TrackSorter::TrackSorter(TrackRepository& repository)
    : repository(repository)
{
}

void TrackSorter::sortByTitle(bool ascending) {
    QList<Track> tracks = repository.findAllTracks();
    std::sort(tracks.begin(), tracks.end(),
              [ascending](const Track& a, const Track& b) {
                  return ascending ? a.getTitle() < b.getTitle() : a.getTitle() > b.getTitle();
              });
    repository.setTracks(tracks);
}

void TrackSorter::sortByArtist(bool ascending) {
    QList<Track> tracks = repository.findAllTracks();
    std::sort(tracks.begin(), tracks.end(),
              [ascending](const Track& a, const Track& b) {
                  return ascending ? a.getArtist() < b.getArtist() : a.getArtist() > b.getArtist();
              });
    repository.setTracks(tracks);
}

void TrackSorter::sortByYear(bool ascending) {
    QList<Track> tracks = repository.findAllTracks();
    std::sort(tracks.begin(), tracks.end(),
              [ascending](const Track& a, const Track& b) {
                  return ascending ? a.getYear() < b.getYear() : a.getYear() > b.getYear();
              });
    repository.setTracks(tracks);
}

void TrackSorter::sortByDuration(bool ascending) {
    QList<Track> tracks = repository.findAllTracks();
    std::sort(tracks.begin(), tracks.end(),
              [ascending](const Track& a, const Track& b) {
                  return ascending ? a.getDuration() < b.getDuration() : a.getDuration() > b.getDuration();
              });
    repository.setTracks(tracks);
}

