// AuthenticationService.h
#ifndef AUTHENTICATIONSERVICE_H
#define AUTHENTICATIONSERVICE_H

#include <QString>

class AuthenticationService {
public:
    static bool validateLogin(const QString& username, const QString& password);
};

#endif // AUTHENTICATIONSERVICE_H


