// YandexMusicAPI.cpp
#include "YandexMusicAPI.h"
#include "Track.h"
#include <QUrlQuery>
#include <QDebug>

YandexMusicAPI::YandexMusicAPI(QObject *parent)
    : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished,
            this, [this](QNetworkReply* reply) {
        if (reply->property("requestType").toString() == "search") {
            onSearchFinished(reply);
        } else if (reply->property("requestType").toString() == "trackInfo") {
            onTrackInfoFinished(reply);
        }
        reply->deleteLater();
    });
}

YandexMusicAPI::~YandexMusicAPI() = default;

void YandexMusicAPI::searchTracks(const QString& query, int page, int limit)
{
    // Используем публичный API Яндекс Музыки для поиска
    // Примечание: это упрощенная версия, для продакшена нужен OAuth токен
    // Альтернативный подход: используем Music API Yandex через публичные эндпоинты
    QString urlString = QString("https://music.yandex.ru/handlers/music-search.jsx?text=%1&type=tracks&page=%2&perPage=%3")
                        .arg(QString::fromUtf8(QUrl::toPercentEncoding(query)))
                        .arg(page)
                        .arg(limit);
    
    QUrl url(urlString);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, 
                     "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
    request.setRawHeader("Accept", "application/json, text/plain, */*");
    request.setRawHeader("Referer", "https://music.yandex.ru/");
    
    QNetworkReply* reply = networkManager->get(request);
    reply->setProperty("requestType", "search");
}

void YandexMusicAPI::getTrackInfo(const QString& trackId)
{
    QString urlString = QString("https://music.yandex.ru/api/v2.1/handlers/track/%1")
                        .arg(trackId);
    
    QUrl url(urlString);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    
    QNetworkReply* reply = networkManager->get(request);
    reply->setProperty("requestType", "trackInfo");
}

void YandexMusicAPI::onSearchFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("Ошибка сети: %1").arg(reply->errorString()));
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        // Попытка парсинга как текстового ответа (Yandex Music может возвращать JSX)
        QString response = QString::fromUtf8(data);
        
        // Ищем JSON в ответе
        int jsonStart = response.indexOf('{');
        
        if (int jsonEnd = response.lastIndexOf('}'); jsonStart >= 0 && jsonEnd > jsonStart) {
            QString jsonString = response.mid(jsonStart, jsonEnd - jsonStart + 1);
            jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);
        }
        
        if (parseError.error != QJsonParseError::NoError) {
            emit errorOccurred(QString("Ошибка парсинга JSON: %1").arg(parseError.errorString()));
            return;
        }
    }

    QList<YandexTrack> tracks = parseSearchResults(jsonDoc);
    emit tracksFound(tracks);
}

void YandexMusicAPI::onTrackInfoFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("Ошибка сети: %1").arg(reply->errorString()));
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred(QString("Ошибка парсинга JSON: %1").arg(parseError.errorString()));
        return;
    }

    YandexTrack track = parseTrackInfo(jsonDoc);
    emit trackInfoReceived(track);
}

namespace {
    QJsonArray extractTracksArray(const QJsonObject& root) {
        if (root.contains("tracks")) {
            QJsonObject tracksObj = root["tracks"].toObject();
            if (tracksObj.contains("results")) {
                return tracksObj["results"].toArray();
            }
            if (tracksObj.contains("items")) {
                return tracksObj["items"].toArray();
            }
        }
        if (root.contains("results")) {
            QJsonObject resultsObj = root["results"].toObject();
            if (resultsObj.contains("tracks")) {
                QJsonObject tracksObj = resultsObj["tracks"].toObject();
                if (tracksObj.contains("results")) {
                    return tracksObj["results"].toArray();
                }
            }
        }
        if (root.contains("items")) {
            return root["items"].toArray();
        }
        return QJsonArray();
    }
    
    void extractAlbumId(YandexTrack& track, const QJsonObject& trackObj) {
        if (trackObj.contains("albums")) {
            QJsonArray albums = trackObj["albums"].toArray();
            if (!albums.isEmpty()) {
                QJsonObject album = albums[0].toObject();
                if (album.contains("id")) {
                    track.albumId = album["id"].toString();
                    return;
                }
            }
        }
        if (trackObj.contains("album")) {
            QJsonObject album = trackObj["album"].toObject();
            if (album.contains("id")) {
                track.albumId = album["id"].toString();
            }
        }
    }
}

QList<YandexTrack> YandexMusicAPI::parseSearchResults(const QJsonDocument& json)
{
    QList<YandexTrack> tracks;
    QJsonObject root = json.object();
    QJsonArray tracksArray = extractTracksArray(root);
    
    for (const QJsonValue& value : tracksArray) {
        QJsonObject trackObj = value.toObject();
        
        YandexTrack track;
        track.id = trackObj["id"].toString();
        extractAlbumId(track, trackObj);
        
        // Название трека
        if (trackObj.contains("title")) {
            track.title = trackObj["title"].toString();
        } else if (trackObj.contains("name")) {
            track.title = trackObj["name"].toString();
        }
        
        // Исполнитель
        track.artist = extractArtist(trackObj);
        
        // Альбом
        track.album = extractAlbum(trackObj);
        
        // Год
        track.year = extractYear(trackObj);
        
        // Жанр
        track.genre = extractGenre(trackObj);
        
        // Длительность
        track.duration = extractDuration(trackObj);
        
        // Обложка
        if (trackObj.contains("coverUri") || trackObj.contains("cover")) {
            QString coverUri = trackObj.contains("coverUri") ? 
                             trackObj["coverUri"].toString() : 
                             trackObj["cover"].toString();
            if (!coverUri.isEmpty()) {
                track.coverUrl = QString("https://%1").arg(coverUri);
            }
        }
        
        if (!track.title.isEmpty() && !track.artist.isEmpty()) {
            tracks.append(track);
        }
    }
    
    return tracks;
}

YandexTrack YandexMusicAPI::parseTrackInfo(const QJsonDocument& json)
{
    YandexTrack track;
    
    if (QJsonObject root = json.object(); root.contains("track")) {
        QJsonObject trackObj = root["track"].toObject();
        
        track.id = trackObj["id"].toString();
        extractAlbumId(track, trackObj);
        
        track.title = trackObj["title"].toString();
        track.artist = extractArtist(trackObj);
        track.album = extractAlbum(trackObj);
        track.year = extractYear(trackObj);
        track.genre = extractGenre(trackObj);
        track.duration = extractDuration(trackObj);
    }
    
    return track;
}

QString YandexMusicAPI::extractArtist(const QJsonObject& trackObj) const
{
    if (trackObj.contains("artists")) {
        QJsonArray artists = trackObj["artists"].toArray();
        if (!artists.isEmpty()) {
            QJsonObject artist = artists[0].toObject();
            if (artist.contains("name")) {
                return artist["name"].toString();
            } else if (artist.contains("title")) {
                return artist["title"].toString();
            }
        }
    } else if (trackObj.contains("artist")) {
        QJsonObject artist = trackObj["artist"].toObject();
        if (artist.contains("name")) {
            return artist["name"].toString();
        }
    } else if (trackObj.contains("artistsNames")) {
        QStringList artists = trackObj["artistsNames"].toString().split(", ");
        if (!artists.isEmpty()) {
            return artists[0];
        }
    }
    
    return "";
}

QString YandexMusicAPI::extractAlbum(const QJsonObject& trackObj) const
{
    if (trackObj.contains("albums")) {
        QJsonArray albums = trackObj["albums"].toArray();
        if (!albums.isEmpty()) {
            QJsonObject album = albums[0].toObject();
            if (album.contains("title")) {
                return album["title"].toString();
            }
        }
    } else if (trackObj.contains("album")) {
        QJsonObject album = trackObj["album"].toObject();
        if (album.contains("title")) {
            return album["title"].toString();
        }
    }
    
    return "";
}

int YandexMusicAPI::extractYear(const QJsonObject& trackObj) const
{
    if (trackObj.contains("albums")) {
        QJsonArray albums = trackObj["albums"].toArray();
        if (!albums.isEmpty()) {
            QJsonObject album = albums[0].toObject();
            if (album.contains("year")) {
                return album["year"].toInt();
            }
        }
    } else if (trackObj.contains("album")) {
        QJsonObject album = trackObj["album"].toObject();
        if (album.contains("year")) {
            return album["year"].toInt();
        }
    } else if (trackObj.contains("year")) {
        return trackObj["year"].toInt();
    }
    
    return 0;
}

QString YandexMusicAPI::extractGenre(const QJsonObject& trackObj) const
{
    if (trackObj.contains("genre")) {
        return trackObj["genre"].toString();
    } else if (trackObj.contains("albums")) {
        QJsonArray albums = trackObj["albums"].toArray();
        if (!albums.isEmpty()) {
            QJsonObject album = albums[0].toObject();
            if (album.contains("genre")) {
                return album["genre"].toString();
            }
        }
    }
    
    return "";
}

int YandexMusicAPI::extractDuration(const QJsonObject& trackObj) const
{
    if (trackObj.contains("durationMs")) {
        return trackObj["durationMs"].toInt() / 1000; // конвертируем миллисекунды в секунды
    } else if (trackObj.contains("duration")) {
        return trackObj["duration"].toInt();
    }
    
    return 0;
}

