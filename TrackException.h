// TrackException.h
#ifndef TRACKEXCEPTION_H
#define TRACKEXCEPTION_H

#include "MusicCatalogException.h"

// Исключение для ошибок работы с треками
class TrackException : public MusicCatalogException {
public:
    explicit TrackException(const QString& message);
    explicit TrackException(int trackId, const QString& operation);
    virtual ~TrackException() noexcept = default;
    
    int getTrackId() const { return trackId; }

protected:
    int trackId;
};

#endif // TRACKEXCEPTION_H

