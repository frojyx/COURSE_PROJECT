// Track.h
#ifndef TRACK_H
#define TRACK_H

#include <QString>
#include <QTextStream>
#include "TXTParser.h"

struct TrackParams {
    QString title;
    QString artist;
    QString album;
    int year;
    QString genre;
    int duration;
    QString filePath;
};

class Track {
public:
    Track();
    Track(int id, const QString& title, const QString& artist,
          const QString& album, int year, const QString& genre, int duration);
    Track(int id, const TrackParams& params);

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

    // Перегрузка операторов сравнения
    bool operator==(const Track& other) const;
    bool operator!=(const Track& other) const;
    bool operator<(const Track& other) const;
    bool operator>(const Track& other) const;
    bool operator<=(const Track& other) const;
    bool operator>=(const Track& other) const;

    // Hidden friend операторы для работы с потоками (C++ Core Guidelines)
    friend QTextStream& operator<<(QTextStream& stream, const Track& track) {
        // Экранируем поля
        QString title = TXTParser::escapeField(track.title);
        QString artist = TXTParser::escapeField(track.artist);
        QString album = TXTParser::escapeField(track.album);
        QString genre = TXTParser::escapeField(track.genre);
        QString filePath = TXTParser::escapeField(track.filePath);

        stream << track.id << TXTParser::FIELD_SEPARATOR
               << title << TXTParser::FIELD_SEPARATOR
               << artist << TXTParser::FIELD_SEPARATOR
               << album << TXTParser::FIELD_SEPARATOR
               << track.year << TXTParser::FIELD_SEPARATOR
               << genre << TXTParser::FIELD_SEPARATOR
               << track.duration << TXTParser::FIELD_SEPARATOR
               << filePath;
        return stream;
    }
    
    friend QTextStream& operator>>(QTextStream& stream, Track& track) {
        QString line = stream.readLine();
        if (line.isEmpty()) {
            return stream;
        }

        QStringList fields = TXTParser::parseLine(line);
        if (fields.size() < 7) {
            return stream;
        }

        bool ok;
        track.id = fields[0].toInt(&ok);
        if (!ok) {
            track.id = 0;
        }

        track.title = TXTParser::unescapeField(fields[1]);
        track.artist = TXTParser::unescapeField(fields[2]);
        track.album = TXTParser::unescapeField(fields[3]);
        track.year = fields[4].toInt(&ok);
        if (!ok) {
            track.year = 0;
        }
        track.genre = TXTParser::unescapeField(fields[5]);
        track.duration = fields[6].toInt(&ok);
        if (!ok) {
            track.duration = 0;
        }
        track.filePath = fields.size() >= 8 ? TXTParser::unescapeField(fields[7]) : "";

        return stream;
    }
    
    // Дружественная функция для сравнения треков по различным критериям
    friend bool compareTracksByTitle(const Track& a, const Track& b);
    friend bool compareTracksByArtist(const Track& a, const Track& b);
    friend bool compareTracksByYear(const Track& a, const Track& b);
    friend bool compareTracksByDuration(const Track& a, const Track& b);

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
