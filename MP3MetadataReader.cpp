// MP3MetadataReader.cpp
#include "MP3MetadataReader.h"
#include <QFile>
#include <QFileInfo>
#include <QByteArray>

// Простая функция для чтения ID3v2 тегов из MP3 файла
QString MP3MetadataReader::readID3v2Tag(const QByteArray& data, const QByteArray& frameId) {
    // Ищем ID3v2 заголовок
    if (data.size() < 10 || !data.startsWith("ID3")) {
        return QString();
    }

    // Пропускаем заголовок ID3v2 (10 байт)
    int pos = 10;

    // Читаем размер тега (4 байта, синхросafe integer)
    int tagSize = ((unsigned char)data[6] << 21) |
                  ((unsigned char)data[7] << 14) |
                  ((unsigned char)data[8] << 7) |
                  (unsigned char)data[9];

    // Ищем нужный фрейм
    while (pos + 10 < data.size() && pos < 10 + tagSize) {
        QByteArray frameHeader = data.mid(pos, 10);
        QByteArray frameID = frameHeader.left(4);

        // Читаем размер фрейма (4 байта, синхросafe integer)
        int frameSize = ((unsigned char)frameHeader[4] << 21) |
                        ((unsigned char)frameHeader[5] << 14) |
                        ((unsigned char)frameHeader[6] << 7) |
                        (unsigned char)frameHeader[7];

        if (frameID == frameId) {
            // Пропускаем флаги (2 байта)
            QByteArray frameData = data.mid(pos + 10, frameSize);

            // Пропускаем encoding byte (обычно UTF-16 или UTF-8)
            if (frameData.size() > 1) {
                int encoding = (unsigned char)frameData[0];
                if (encoding == 0 || encoding == 3) {
                    // ISO-8859-1 или UTF-8
                    QString result = QString::fromUtf8(frameData.mid(1));
                    return result.trimmed();
                } else if (encoding == 1 || encoding == 2) {
                    // UTF-16 with BOM или UTF-16BE
                    const char16_t* data = reinterpret_cast<const char16_t*>(frameData.constData() + 1);
                    qsizetype length = (frameSize - 1) / 2;
                    QString result = QString::fromUtf16(data, length);
                    return result.trimmed();
                }
            }
        }

        // Переходим к следующему фрейму
        pos += 10 + frameSize;
    }

    return QString();
}

// Функция для вычисления длительности MP3 файла
int MP3MetadataReader::calculateMP3Duration(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return 0;
    }

    qint64 fileSize = QFileInfo(filePath).size();

    // Читаем файл по частям, ищем MP3 фреймы
    QByteArray buffer = file.read(100000);
    file.close();

    // Ищем MP3 фрейм (синхронное слово 0xFFE0-0xFFEF)
    for (int i = 0; i < buffer.size() - 4; ++i) {
        if ((unsigned char)buffer[i] == 0xFF &&
            ((unsigned char)buffer[i+1] & 0xE0) == 0xE0) {

            // Читаем заголовок MP3 фрейма
            unsigned char header2 = buffer[i+1];
            unsigned char header3 = buffer[i+2];

            // Проверяем версию MP3 (MPEG-1, MPEG-2)
            int layer = (header2 >> 1) & 0x03;

            if (layer == 1) { // Layer III (MP3)
                // Извлекаем битрейт
                int bitrateIndex = (header3 >> 4) & 0x0F;
                // Извлекаем частоту дискретизации
                int sampleRateIndex = (header3 >> 2) & 0x03;

                // Битрейт для MPEG-1 Layer III (kbps)
                int bitrates[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
                if (bitrateIndex == 0 || bitrateIndex >= 15) {
                    continue;
                }
                int bitrate = bitrates[bitrateIndex] * 1000; // в битах в секунду

                // Частота дискретизации для MPEG-1 (Hz)
                int sampleRates[] = {44100, 48000, 32000, 0};
                if (sampleRateIndex >= 3) {
                    continue;
                }
                int sampleRate = sampleRates[sampleRateIndex];

                if (bitrate > 0 && sampleRate > 0) {
                    // Длительность = (размер файла * 8) / битрейт
                    int duration = (fileSize * 8) / bitrate;
                    return duration;
                }
            }
        }
    }

    return 0;
}

bool MP3MetadataReader::readMP3Metadata(const QString& filePath, QString& title, QString& artist,
                                         QString* album, int* year, QString* genre, int* duration) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // Читаем первые 64KB для поиска ID3 тегов
    QByteArray data = file.read(65536);
    file.close();

    if (data.size() < 10) {
        return false;
    }

    // Пытаемся прочитать ID3v2 теги
    QString id3Title = readID3v2Tag(data, "TIT2"); // TIT2 = Title
    QString id3Artist = readID3v2Tag(data, "TPE1"); // TPE1 = Lead artist
    QString id3Album = readID3v2Tag(data, "TALB"); // TALB = Album
    QString id3Year = readID3v2Tag(data, "TDRC"); // TDRC = Year (Recording time)
    QString id3Genre = readID3v2Tag(data, "TCON"); // TCON = Genre

    // Если не нашли год в TDRC, пробуем TYER (старый формат)
    if (id3Year.isEmpty()) {
        id3Year = readID3v2Tag(data, "TYER"); // TYER = Year
    }

    // Если не нашли, пробуем старые ID3v1 теги (в конце файла)
    if (id3Title.isEmpty() || id3Artist.isEmpty() || id3Album.isEmpty() || id3Year.isEmpty()) {
        file.open(QIODevice::ReadOnly);
        if (file.seek(file.size() - 128)) {
            QByteArray id3v1 = file.read(128);
            if (id3v1.startsWith("TAG")) {
                if (id3Title.isEmpty()) {
                    id3Title = QString::fromLatin1(id3v1.mid(3, 30)).trimmed();
                }
                if (id3Artist.isEmpty()) {
                    id3Artist = QString::fromLatin1(id3v1.mid(33, 30)).trimmed();
                }
                if (id3Album.isEmpty()) {
                    id3Album = QString::fromLatin1(id3v1.mid(63, 30)).trimmed();
                }
                if (id3Year.isEmpty()) {
                    id3Year = QString::fromLatin1(id3v1.mid(93, 4)).trimmed();
                }
                if (id3Genre.isEmpty()) {
                    unsigned char genreByte = id3v1[125];
                    // Базовый список жанров ID3v1 (первые 80)
                    if (genreByte < 80) {
                        QString genres[] = {"Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk", "Grunge",
                                            "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies", "Other", "Pop", "R&B",
                                            "Rap", "Reggae", "Rock", "Techno", "Industrial", "Alternative", "Ska",
                                            "Death Metal", "Pranks", "Soundtrack", "Euro-Techno", "Ambient",
                                            "Trip-Hop", "Vocal", "Jazz+Funk", "Fusion", "Trance", "Classical",
                                            "Instrumental", "Acid", "House", "Game", "Sound Clip", "Gospel", "Noise",
                                            "AlternRock", "Bass", "Soul", "Punk", "Space", "Meditative",
                                            "Instrumental Pop", "Instrumental Rock", "Ethnic", "Gothic", "Darkwave",
                                            "Techno-Industrial", "Electronic", "Pop-Folk", "Eurodance", "Dream",
                                            "Southern Rock", "Comedy", "Cult", "Gangsta", "Top 40", "Christian Rap",
                                            "Pop/Funk", "Jungle", "Native American", "Cabaret", "New Wave",
                                            "Psychadelic", "Rave", "Showtunes", "Trailer", "Lo-Fi", "Tribal",
                                            "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical", "Rock & Roll",
                                            "Hard Rock"};
                        id3Genre = genres[genreByte];
                    }
                }
            }
        }
        file.close();
    }

    // Заполняем поля
    bool found = false;
    if (!id3Title.isEmpty()) {
        title = id3Title;
        found = true;
    }
    if (!id3Artist.isEmpty()) {
        artist = id3Artist;
        found = true;
    }
    if (!id3Album.isEmpty() && album != nullptr) {
        *album = id3Album;
    }
    if (!id3Year.isEmpty() && year != nullptr) {
        bool ok;
        int yearValue = id3Year.left(4).toInt(&ok);
        if (ok && yearValue > 1900 && yearValue < 2100) {
            *year = yearValue;
        }
    }
    if (!id3Genre.isEmpty() && genre != nullptr) {
        *genre = id3Genre;
    }

    // Пытаемся вычислить длительность из MP3 заголовков
    if (duration != nullptr) {
        int mp3Duration = calculateMP3Duration(filePath);
        if (mp3Duration > 0) {
            *duration = mp3Duration;
        }
    }

    return found;
}

