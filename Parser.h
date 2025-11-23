// Parser.h
#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QStringList>

class Parser {
public:
    virtual ~Parser() = default;

    virtual QString escapeField(const QString& field) const = 0;
    virtual QStringList parseLine(const QString& line) const = 0;
    virtual QString unescapeField(const QString& field) const = 0;
};

#endif // PARSER_H


