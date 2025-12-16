// ParseException.h
#ifndef PARSEEXCEPTION_H
#define PARSEEXCEPTION_H

#include "exceptions/MusicCatalogException.h"

// Исключение для ошибок парсинга
class ParseException : public MusicCatalogException {
public:
    explicit ParseException(const QString& message);
    explicit ParseException(const QString& data, const QString& expectedFormat);
    explicit ParseException(int lineNumber, const QString& error);
    ~ParseException() noexcept override = default;
    
    int getLineNumber() const { return lineNumber; }

private:
    int lineNumber = -1;
};

#endif // PARSEEXCEPTION_H

