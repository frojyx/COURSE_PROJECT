#include "TrackSorter.h"
#include "TrackRepository.h"
#include "Track.h"

TrackSorter::TrackSorter(TrackRepository& repository)
    : repository(repository)
{
}

template <typename T, typename Compare>
static void shellSort(QList<T>& list, Compare comp)
{
    int n = list.size();
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            T temp = list[i];
            int j;
            for (j = i; j >= gap && comp(temp, list[j - gap]); j -= gap) {
                list[j] = list[j - gap];
            }
            list[j] = temp;
        }
    }
}

void TrackSorter::sortByTitle(bool ascending) {
    QList<Track> tracks = repository.findAllTracks();

    auto cmp = [ascending](const Track& a, const Track& b){
        return ascending ? a.getTitle() < b.getTitle()
                         : a.getTitle() > b.getTitle();
    };

    shellSort(tracks, cmp);
    repository.setTracks(tracks);
}

void TrackSorter::sortByArtist(bool ascending) {
    QList<Track> tracks = repository.findAllTracks();

    auto cmp = [ascending](const Track& a, const Track& b){
        return ascending ? a.getArtist() < b.getArtist()
                         : a.getArtist() > b.getArtist();
    };

    shellSort(tracks, cmp);
    repository.setTracks(tracks);
}

void TrackSorter::sortByYear(bool ascending) {
    QList<Track> tracks = repository.findAllTracks();

    auto cmp = [ascending](const Track& a, const Track& b){
        return ascending ? a.getYear() < b.getYear()
                         : a.getYear() > b.getYear();
    };

    shellSort(tracks, cmp);
    repository.setTracks(tracks);
}

void TrackSorter::sortByDuration(bool ascending) {
    QList<Track> tracks = repository.findAllTracks();

    auto cmp = [ascending](const Track& a, const Track& b){
        return ascending ? a.getDuration() < b.getDuration()
                         : a.getDuration() > b.getDuration();
    };

    shellSort(tracks, cmp);
    repository.setTracks(tracks);
}
