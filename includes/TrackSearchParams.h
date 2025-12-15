// TrackSearchParams.h
#ifndef TRACKSEARCHPARAMS_H
#define TRACKSEARCHPARAMS_H

#include <QString>

// Структура для параметров поиска треков
struct TrackSearchParams {
    QString title;
    QString artist;
    QString album;
    QString genre;
    int minYear = 0;
    int maxYear = 0;
    int minDuration = 0;
    int maxDuration = 0;
};

// Структура для параметров добавления трека
struct TrackAddParams {
    QString title;
    QString artist;
    QString album;
    int year = 0;
    QString genre;
    int duration = 0;
    QString filePath;
};

#endif // TRACKSEARCHPARAMS_H

