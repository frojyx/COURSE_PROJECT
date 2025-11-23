// MusicCatalogException.h
#ifndef MUSICCATALOGEXCEPTION_H
#define MUSICCATALOGEXCEPTION_H

#include <exception>
#include <QString>

// Базовый класс для всех исключений в приложении
class MusicCatalogException : public std::exception {
public:
    explicit MusicCatalogException(const QString& message);
    virtual ~MusicCatalogException() noexcept = default;

    // Возвращает сообщение об ошибке
    const char* what() const noexcept override;
    QString getMessage() const;

protected:
    QString errorMessage;
};

#endif // MUSICCATALOGEXCEPTION_H

