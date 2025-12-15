// TrackException.h
#ifndef TRACKEXCEPTION_H
#define TRACKEXCEPTION_H

#include "MusicCatalogException.h"

// Исключение для ошибок работы с треками
class TrackException : public MusicCatalogException {
public:
    explicit TrackException(const QString& message);
    explicit TrackException(int trackId, const QString& operation);
    ~TrackException() noexcept override = default;
    
    int getTrackId() const { return trackId; }

private:
    int trackId = -1;
};

#endif // TRACKEXCEPTION_H

