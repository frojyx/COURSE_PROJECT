// YandexMusicAPI.h
#ifndef YANDEXMUSICAPI_H
#define YANDEXMUSICAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QString>

class Track;
class MusicCatalog;

struct YandexTrack {
    QString id;
    QString albumId; // ID альбома для формирования ссылки
    QString title;
    QString artist;
    QString album;
    int year;
    QString genre;
    int duration; // в секундах
    QString coverUrl;
};

class YandexMusicAPI : public QObject
{
    Q_OBJECT

public:
    explicit YandexMusicAPI(QObject *parent = nullptr);
    ~YandexMusicAPI();

    // Поиск треков
    void searchTracks(const QString& query, int page = 0, int limit = 20);
    
    // Получение информации о треке по ID
    void getTrackInfo(const QString& trackId);

signals:
    // Сигналы для уведомления о результатах
    void tracksFound(const QList<YandexTrack>& tracks);
    void trackInfoReceived(const YandexTrack& track);
    void errorOccurred(const QString& errorMessage);

private slots:
    void onSearchFinished(QNetworkReply* reply);
    void onTrackInfoFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* networkManager;
    
    // Парсинг JSON ответов
    QList<YandexTrack> parseSearchResults(const QJsonDocument& json);
    YandexTrack parseTrackInfo(const QJsonDocument& json);
    
    // Вспомогательные методы
    QString extractArtist(const QJsonObject& trackObj);
    QString extractAlbum(const QJsonObject& trackObj);
    int extractYear(const QJsonObject& trackObj);
    QString extractGenre(const QJsonObject& trackObj);
    int extractDuration(const QJsonObject& trackObj);
};

#endif // YANDEXMUSICAPI_H

