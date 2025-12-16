// MusicCatalogException.cpp
#include "exceptions/MusicCatalogException.h"
#include <QByteArray>

MusicCatalogException::MusicCatalogException(const QString& message)
    : errorMessage(message)
{
}

const char* MusicCatalogException::what() const noexcept {
    static QByteArray ba;
    ba = errorMessage.toUtf8();
    return ba.constData();
}

QString MusicCatalogException::getMessage() const {
    return errorMessage;
}

