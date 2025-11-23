// CSVParser.h
#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <QString>
#include <QStringList>

class CSVParser {
public:
    // Экранирование поля для CSV
    static QString escapeField(const QString& field);

    // Распарсить строку CSV с учетом кавычек
    static QStringList parseLine(const QString& line);

    // Убрать кавычки из поля
    static QString unescapeField(const QString& field);
};

#endif // CSVPARSER_H


