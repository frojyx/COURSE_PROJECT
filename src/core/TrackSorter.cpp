#include "core/TrackSorter.h"
#include "core/TrackRepository.h"
#include "core/Track.h"

// Используем дружественные функции для сравнения треков
extern bool compareTracksByTitle(const Track& a, const Track& b);
extern bool compareTracksByArtist(const Track& a, const Track& b);
extern bool compareTracksByYear(const Track& a, const Track& b);
extern bool compareTracksByDuration(const Track& a, const Track& b);

TrackSorter::TrackSorter(TrackRepository& repository)
    : repository(repository)
{
}

template <typename T, typename Compare>
static void shellSort(QList<T>& list, Compare comp)
{
    const int n = list.size();

    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {

            T temp = list[i];
            int j = i;

            while (j >= gap && comp(temp, list[j - gap])) {
                list[j] = list[j - gap];
                j -= gap;
            }

            list[j] = temp;
        }
    }
}

    void TrackSorter::sortByTitle(bool ascending) {
        QList<Track> tracks = repository.findAllTracks();

        auto cmp = [ascending](const Track& a, const Track& b){
            const bool result = compareTracksByTitle(a, b);
            return ascending ? result : !result;
        };

        shellSort(tracks, cmp);
        repository.setTracks(tracks);
    }

    void TrackSorter::sortByArtist(bool ascending) {
        QList<Track> tracks = repository.findAllTracks();

        auto cmp = [ascending](const Track& a, const Track& b){
            const bool result = compareTracksByArtist(a, b);
            return ascending ? result : !result;
        };

        shellSort(tracks, cmp);
        repository.setTracks(tracks);
    }

    void TrackSorter::sortByYear(bool ascending) {
        QList<Track> tracks = repository.findAllTracks();

        auto cmp = [ascending](const Track& a, const Track& b){
            const bool result = compareTracksByYear(a, b);
            return ascending ? result : !result;
        };

        shellSort(tracks, cmp);
        repository.setTracks(tracks);
    }

    void TrackSorter::sortByDuration(bool ascending) {
        QList<Track> tracks = repository.findAllTracks();

        auto cmp = [ascending](const Track& a, const Track& b){
            const bool result = compareTracksByDuration(a, b);
            return ascending ? result : !result;
        };

        shellSort(tracks, cmp);
        repository.setTracks(tracks);
    }
