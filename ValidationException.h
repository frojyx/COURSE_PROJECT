// ValidationException.h
#ifndef VALIDATIONEXCEPTION_H
#define VALIDATIONEXCEPTION_H

#include "MusicCatalogException.h"

// Исключение для ошибок валидации данных
class ValidationException : public MusicCatalogException {
public:
    explicit ValidationException(const QString& message);
    explicit ValidationException(const QString& fieldName, const QString& reason);
    virtual ~ValidationException() noexcept = default;
    
    QString getFieldName() const { return fieldName; }

protected:
    QString fieldName;
};

#endif // VALIDATIONEXCEPTION_H

