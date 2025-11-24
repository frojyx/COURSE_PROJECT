// YandexMusicIntegrator.cpp
#include "YandexMusicIntegrator.h"
#include "Track.h"
#include <QDebug>
#include <QUrl>

YandexMusicIntegrator::YandexMusicIntegrator(MusicCatalog* catalog, QObject *parent)
    : QObject(parent), catalog(catalog)
{
    api = new YandexMusicAPI(this);
    
    connect(api, &YandexMusicAPI::tracksFound,
            this, &YandexMusicIntegrator::onTracksFound);
    connect(api, &YandexMusicAPI::errorOccurred,
            this, &YandexMusicIntegrator::onErrorOccurred);
}

void YandexMusicIntegrator::searchAndImportTracks(const QString& query)
{
    api->searchTracks(query);
}

void YandexMusicIntegrator::onTracksFound(const QList<YandexTrack>& tracks)
{
    emit tracksFound(tracks);
}

void YandexMusicIntegrator::importTrack(const YandexTrack& track)
{
    if (track.title.isEmpty() || track.artist.isEmpty()) {
        emit errorOccurred("Трек не содержит необходимой информации");
        return;
    }
    
    // Преобразуем год (если 0, используем текущий год или оставляем 0)
    int year = track.year > 0 ? track.year : 0;
    
    // Преобразуем жанр (если пустой, используем "Неизвестно")
    QString genre = track.genre.isEmpty() ? "Неизвестно" : track.genre;
    
    // Формируем ссылку на трек в Яндекс Музыке
    QString yandexUrl;
    if (!track.id.isEmpty()) {
        if (!track.albumId.isEmpty()) {
            // Формируем полную ссылку с альбомом: https://music.yandex.ru/album/{albumId}/track/{trackId}
            yandexUrl = QString("https://music.yandex.ru/album/%1/track/%2")
                       .arg(track.albumId)
                       .arg(track.id);
        } else {
            // Если нет albumId, используем только trackId (но это может не работать)
            // Лучше использовать поиск
            QString searchQuery = QString("%1 %2").arg(track.artist, track.title);
            yandexUrl = QString("https://music.yandex.ru/search?text=%1")
                       .arg(QString::fromUtf8(QUrl::toPercentEncoding(searchQuery)));
        }
    } else {
        // Если нет ID, формируем поисковый запрос
        QString searchQuery = QString("%1 %2").arg(track.artist, track.title);
        yandexUrl = QString("https://music.yandex.ru/search?text=%1")
                   .arg(QString::fromUtf8(QUrl::toPercentEncoding(searchQuery)));
    }
    
    // Добавляем трек в каталог с сохранением ссылки на Яндекс Музыку
    catalog->addTrack(track.title,
                     track.artist,
                     track.album,
                     year,
                     genre,
                     track.duration,
                     yandexUrl); // Сохраняем ссылку на Яндекс Музыку
    
    // Автоматическое сохранение
    autoSave();
    
    emit trackImported(track.title);
}

void YandexMusicIntegrator::onErrorOccurred(const QString& errorMessage)
{
    emit errorOccurred(errorMessage);
}

void YandexMusicIntegrator::autoSave()
{
    // Автоматическое сохранение каталога
    QString fileName = "catalog_autosave.txt";
    FileManager::saveToTXT(*catalog, fileName);
}

