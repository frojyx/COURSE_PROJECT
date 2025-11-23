// FileException.cpp
#include "FileException.h"

FileException::FileException(const QString& message)
    : MusicCatalogException(message)
{
}

FileException::FileException(const QString& filename, const QString& operation)
    : MusicCatalogException(QString("Ошибка при %1 файла '%2'").arg(operation, filename))
{
}

