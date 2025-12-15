// TrackSorter.h
#ifndef TRACKSORTER_H
#define TRACKSORTER_H

class TrackRepository;

class TrackSorter {
public:
    explicit TrackSorter(TrackRepository& repository);

    // Сортировка
    void sortByTitle(bool ascending = true);
    void sortByArtist(bool ascending = true);
    void sortByYear(bool ascending = true);
    void sortByDuration(bool ascending = true);

private:
    TrackRepository& repository;
};

#endif // TRACKSORTER_H

