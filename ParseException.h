// ParseException.h
#ifndef PARSEEXCEPTION_H
#define PARSEEXCEPTION_H

#include "MusicCatalogException.h"

// Исключение для ошибок парсинга
class ParseException : public MusicCatalogException {
public:
    explicit ParseException(const QString& message);
    explicit ParseException(const QString& data, const QString& expectedFormat);
    explicit ParseException(int lineNumber, const QString& error);
    virtual ~ParseException() noexcept = default;
    
    int getLineNumber() const { return lineNumber; }

protected:
    int lineNumber;
};

#endif // PARSEEXCEPTION_H

