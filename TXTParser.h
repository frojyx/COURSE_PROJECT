// TXTParser.h
#ifndef TXTPARSER_H
#define TXTPARSER_H

#include <QString>
#include <QStringList>

class TXTParser {
public:
    // Разделитель полей
    static const QString FIELD_SEPARATOR;
    
    // Экранирование поля для TXT (замена разделителя на экранированную версию)
    static QString escapeField(const QString& field);
    
    // Распарсить строку TXT с учетом экранирования
    static QStringList parseLine(const QString& line);
    
    // Убрать экранирование из поля
    static QString unescapeField(const QString& field);
};

#endif // TXTPARSER_H

