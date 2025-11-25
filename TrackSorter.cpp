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
    auto comparator = [ascending](const Track& a, const Track& b) {
        return ascending ? a.getTitle() < b.getTitle()
                         : a.getTitle() > b.getTitle();
    };
    std::stable_sort(tracks.begin(), tracks.end(), comparator);
    repository.setTracks(tracks);
}

void TrackSorter::sortByArtist(bool ascending) {
    QList<Track> tracks = repository.findAllTracks();
    auto comparator = [ascending](const Track& a, const Track& b) {
        return ascending ? a.getArtist() < b.getArtist()
                         : a.getArtist() > b.getArtist();
    };
    std::stable_sort(tracks.begin(), tracks.end(), comparator);
    repository.setTracks(tracks);
}

void TrackSorter::sortByYear(bool ascending) {
    QList<Track> tracks = repository.findAllTracks();
    auto comparator = [ascending](const Track& a, const Track& b) {
        return ascending ? a.getYear() < b.getYear()
                         : a.getYear() > b.getYear();
    };
    std::stable_sort(tracks.begin(), tracks.end(), comparator);
    repository.setTracks(tracks);
}

void TrackSorter::sortByDuration(bool ascending) {
    QList<Track> tracks = repository.findAllTracks();
    auto comparator = [ascending](const Track& a, const Track& b) {
        return ascending ? a.getDuration() < b.getDuration()
                         : a.getDuration() > b.getDuration();
    };
    std::stable_sort(tracks.begin(), tracks.end(), comparator);
    repository.setTracks(tracks);
}
