// TrackException.cpp
#include "TrackException.h"

TrackException::TrackException(const QString& message)
    : MusicCatalogException(message)
{
}

TrackException::TrackException(int trackId, const QString& operation)
    : MusicCatalogException(QString("Ошибка при %1 трека с ID %2").arg(operation).arg(trackId)), trackId(trackId)
{
}

