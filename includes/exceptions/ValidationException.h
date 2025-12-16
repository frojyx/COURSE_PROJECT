// ValidationException.h
#ifndef VALIDATIONEXCEPTION_H
#define VALIDATIONEXCEPTION_H

#include "exceptions/MusicCatalogException.h"

// Исключение для ошибок валидации данных
class ValidationException : public MusicCatalogException {
public:
    explicit ValidationException(const QString& message);
    explicit ValidationException(const QString& fieldName, const QString& reason);
    ~ValidationException() noexcept override = default;
    
    QString getFieldName() const { return fieldName; }

private:
    QString fieldName;
};

#endif // VALIDATIONEXCEPTION_H

