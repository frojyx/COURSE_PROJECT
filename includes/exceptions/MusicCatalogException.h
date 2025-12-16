// MusicCatalogException.h
#ifndef MUSICCATALOGEXCEPTION_H
#define MUSICCATALOGEXCEPTION_H

#include <exception>
#include <QString>

// Базовый класс для всех исключений в приложении
class MusicCatalogException : public std::exception {
public:
    explicit MusicCatalogException(const QString& message);
    ~MusicCatalogException() noexcept override = default;

    // Возвращает сообщение об ошибке
    const char* what() const noexcept override;
    QString getMessage() const;

private:
    QString errorMessage;
};

#endif // MUSICCATALOGEXCEPTION_H

