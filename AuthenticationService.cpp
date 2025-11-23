// AuthenticationService.cpp
#include "AuthenticationService.h"

bool AuthenticationService::validateLogin(const QString& username, const QString& password) {
    return username == "guerra" && password == "2007";
}


