// TXTParser.cpp
#include "TXTParser.h"

const QString TXTParser::FIELD_SEPARATOR = "|||";

QString TXTParser::escapeField(const QString& field) {
    QString escaped = field;
    // Сначала экранируем обратный слэш (чтобы не экранировать уже экранированные символы)
    escaped.replace("\\", "\\\\");
    // Затем экранируем переводы строк
    escaped.replace("\n", "\\n");
    escaped.replace("\r", "\\r");
    // В последнюю очередь экранируем разделитель
    escaped.replace("|||", "\\|||");
    return escaped;
}

QStringList TXTParser::parseLine(const QString& line) {
    QStringList fields;
    QString field;
    bool escapeNext = false;

    // Парсинг TXT с учетом экранирования
    for (int i = 0; i < line.length(); ++i) {
        QChar c = line[i];
        
        if (escapeNext) {
            // Обрабатываем экранированный символ
            if (c == '|') {
                // Проверяем, не является ли это экранированным разделителем |||
                if (i + 1 < line.length() && line[i + 1] == '|' && i + 2 < line.length() && line[i + 2] == '|') {
                    // Это экранированный разделитель |||
                    field += "|||";
                    i += 2; // Пропускаем следующие две |
                } else {
                    field += '|';
                }
            } else if (c == 'n') {
                field += '\n';
            } else if (c == 'r') {
                field += '\r';
            } else if (c == '\\') {
                field += '\\';
            } else {
                // Неизвестный экранированный символ - добавляем как есть
                field += '\\';
                field += c;
            }
            escapeNext = false;
        } else if (c == '\\') {
            escapeNext = true;
        } else {
            // Проверяем, не является ли это разделителем
            if (c == '|' && i + 1 < line.length() && line[i + 1] == '|' && i + 2 < line.length() && line[i + 2] == '|') {
                // Найден разделитель |||
                fields.append(field);
                field.clear();
                i += 2; // Пропускаем следующие две |
            } else {
                field.append(c);
            }
        }
    }
    
    // Если строка закончилась на экранирующий символ
    if (escapeNext) {
        field += '\\';
    }
    fields.append(field); // Добавляем последнее поле

    return fields;
}

QString TXTParser::unescapeField(const QString& field) {
    // parseLine уже обработал все экранирование, поэтому просто возвращаем поле как есть
    // Этот метод оставлен для совместимости с существующим кодом
    return field;
}

