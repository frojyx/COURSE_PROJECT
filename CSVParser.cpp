// CSVParser.cpp
#include "CSVParser.h"

QString CSVParser::escapeField(const QString& field) {
    // Если поле содержит запятые или кавычки, заключаем в кавычки
    if (field.contains(',') || field.contains('"') || field.contains('\n')) {
        QString escaped = field;
        escaped.replace("\"", "\"\"");
        return "\"" + escaped + "\"";
    }
    return field;
}

QStringList CSVParser::parseLine(const QString& line) {
    QStringList fields;
    QString field;
    bool inQuotes = false;

    // Парсинг CSV с учетом кавычек
    for (int i = 0; i < line.length(); ++i) {
        QChar c = line[i];

        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.append(field);
            field.clear();
        } else {
            field.append(c);
        }
    }
    fields.append(field); // Добавляем последнее поле

    return fields;
}

QString CSVParser::unescapeField(const QString& field) {
    QString result = field;
    if (result.startsWith('"') && result.endsWith('"')) {
        result = result.mid(1, result.length() - 2).replace("\"\"", "\"");
    }
    return result;
}

