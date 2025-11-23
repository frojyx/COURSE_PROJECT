// Track.h
#ifndef TRACK_H
#define TRACK_H

#include <QString>

class Track {
public:
    Track();
    Track(int id, const QString& title, const QString& artist,
          const QString& album, int year, const QString& genre, int duration);
    Track(int id, const QString& title, const QString& artist,
          const QString& album, int year, const QString& genre, int duration, const QString& filePath);

    // Геттеры
    int getId() const { return id; }
    QString getTitle() const { return title; }
    QString getArtist() const { return artist; }
    QString getAlbum() const { return album; }
    int getYear() const { return year; }
    QString getGenre() const { return genre; }
    int getDuration() const { return duration; }
    QString getFormattedDuration() const;
    QString getFilePath() const { return filePath; }

    // Сеттеры
    void setId(int newId) { id = newId; }
    void setTitle(const QString& newTitle) { title = newTitle; }
    void setArtist(const QString& newArtist) { artist = newArtist; }
    void setAlbum(const QString& newAlbum) { album = newAlbum; }
    void setYear(int newYear) { year = newYear; }
    void setGenre(const QString& newGenre) { genre = newGenre; }
    void setDuration(int newDuration) { duration = newDuration; }
    void setFilePath(const QString& newFilePath) { filePath = newFilePath; }

    // Вспомогательные методы
    bool matchesSearch(const QString& searchTerm) const;

private:
    int id;
    QString title;
    QString artist;
    QString album;
    int year;
    QString genre;
    int duration;
    QString filePath; // Путь к MP3 файлу
};

#endif // TRACK_H
