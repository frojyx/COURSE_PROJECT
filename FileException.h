// FileException.h
#ifndef FILEEXCEPTION_H
#define FILEEXCEPTION_H

#include "MusicCatalogException.h"

// Исключение для ошибок работы с файлами
class FileException : public MusicCatalogException {
public:
    explicit FileException(const QString& message);
    explicit FileException(const QString& filename, const QString& operation);
    virtual ~FileException() noexcept = default;
};

#endif // FILEEXCEPTION_H

