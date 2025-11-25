// TXTParser.cpp
#include "TXTParser.h"

const QString TXTParser::FIELD_SEPARATOR = "|||";

QString TXTParser::escapeField(const QString& field) {
    QString escaped = field;
    // Сначала экранируем обратный слэш (чтобы не экранировать уже экранированные символы)
    escaped.replace(R"(\)", R"(\\)");
    // Затем экранируем переводы строк
    escaped.replace("\n", "\\n");
    escaped.replace("\r", "\\r");
    // В последнюю очередь экранируем разделитель
    escaped.replace("|||", "\\|||");
    return escaped;
}

namespace {
    bool isSeparatorAt(const QString& line, int pos) {
        const int lineLen = line.length();
        return pos < lineLen && line[pos] == '|' &&
               pos + 1 < lineLen && line[pos + 1] == '|' &&
               pos + 2 < lineLen && line[pos + 2] == '|';
    }
    
    void handleEscapedChar(QChar c, QString& field, int& i, const QString& line) {
        if (c == '|' && isSeparatorAt(line, i)) {
            field += "|||";
            i += 2;
        } else if (c == 'n') {
            field += '\n';
        } else if (c == 'r') {
            field += '\r';
        } else if (c == '\\') {
            field += '\\';
        } else {
            field += '\\';
            field += c;
        }
    }
}

QStringList TXTParser::parseLine(const QString& line) {
    QStringList fields;
    QString field;
    bool escapeNext = false;

    // Парсинг TXT с учетом экранирования
    const int lineLen = line.length();
    for (int i = 0; i < lineLen; ++i) {
        QChar c = line[i];
        
        if (escapeNext) {
            handleEscapedChar(c, field, i, line);
            escapeNext = false;
            continue;
        }
        
        if (c == '\\') {
            escapeNext = true;
            continue;
        }
        
        if (isSeparatorAt(line, i)) {
            fields.append(field);
            field.clear();
            i += 2;
        } else {
            field.append(c);
        }
    }
    
    if (escapeNext) {
        field += '\\';
    }
    fields.append(field);

    return fields;
}

QString TXTParser::unescapeField(const QString& field) {
    // parseLine уже обработал все экранирование, поэтому просто возвращаем поле как есть
    // Этот метод оставлен для совместимости с существующим кодом
    return field;
}

