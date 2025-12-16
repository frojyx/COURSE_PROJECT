// YandexMusicIntegrator.h
#ifndef YANDEXMUSICINTEGRATOR_H
#define YANDEXMUSICINTEGRATOR_H

#include "integration/YandexMusicAPI.h"
#include "core/MusicCatalog.h"
#include "file_operations/FileManager.h"
#include <QObject>

class YandexMusicIntegrator : public QObject
{
    Q_OBJECT

public:
    explicit YandexMusicIntegrator(MusicCatalog* catalog, QObject *parent = nullptr);
    
    // Поиск и импорт треков
    void searchAndImportTracks(const QString& query);
    
    // Импорт конкретного трека
    void importTrack(const YandexTrack& track);

signals:
    void tracksFound(const QList<YandexTrack>& tracks);
    void trackImported(const QString& trackTitle);
    void importFinished(int importedCount);
    void errorOccurred(const QString& errorMessage);

private slots:
    void onTracksFound(const QList<YandexTrack>& tracks);
    void onErrorOccurred(const QString& errorMessage);

private:
    YandexMusicAPI* api;
    MusicCatalog* catalog;
    
    // Автоматическое сохранение после импорта
    void autoSave() const;
};

#endif // YANDEXMUSICINTEGRATOR_H

