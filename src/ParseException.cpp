// ParseException.cpp
#include "ParseException.h"

ParseException::ParseException(const QString& message)
    : MusicCatalogException(message)
{
}

ParseException::ParseException(const QString& [[maybe_unused]] data, const QString& expectedFormat)
    : MusicCatalogException(QString("Ошибка парсинга: неверный формат данных. Ожидался: %1").arg(expectedFormat))
{
}

ParseException::ParseException(int lineNumber, const QString& error)
    : MusicCatalogException(QString("Ошибка парсинга на строке %1: %2").arg(lineNumber).arg(error)),
      lineNumber(lineNumber)
{
}

