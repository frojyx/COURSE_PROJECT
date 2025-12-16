// ValidationException.cpp
#include "exceptions/ValidationException.h"

ValidationException::ValidationException(const QString& message)
    : MusicCatalogException(message)
{
}

ValidationException::ValidationException(const QString& fieldName, const QString& reason)
    : MusicCatalogException(QString("Ошибка валидации поля '%1': %2").arg(fieldName, reason)),
      fieldName(fieldName)
{
}

