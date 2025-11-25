// MP3MetadataReader.cpp
#include "MP3MetadataReader.h"
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QVector>
#include <cstddef>
#include <array>
#include <cstdint>
#include <cstring>
#include <bit>

namespace {
    // Безопасное преобразование UTF-16 данных из ID3 фрейма
    QString convertUTF16Frame(const QByteArray& frameData) {
        if (frameData.size() <= 1) {
            return QString();
        }
        
        QByteArray utf16Data = frameData.mid(1);
        if (utf16Data.size() % 2 != 0) {
            return QString();
        }
        
        // Используем QByteArray для безопасного преобразования без reinterpret_cast
        // Копируем данные в выровненный буфер и используем QString::fromUtf16
        if (const int charCount = utf16Data.size() / 2; charCount > 0) {
            // Создаем временный массив char16_t для безопасного преобразования
            QVector<char16_t> utf16Buffer(charCount);
            std::memcpy(utf16Buffer.data(), utf16Data.constData(), utf16Data.size());
            return QString::fromUtf16(utf16Buffer.constData(), charCount);
        }
        return QString();
    }
    
    // Обработка кодировки фрейма
    QString processFrameEncoding(const QByteArray& frameData, int encoding) {
        if (encoding == 0 || encoding == 3) {
            // ISO-8859-1 или UTF-8
            return QString::fromUtf8(frameData.mid(1));
        }
        
        if (encoding == 1 || encoding == 2) {
            // UTF-16 with BOM или UTF-16BE
            return convertUTF16Frame(frameData);
        }
        
        return QString();
    }
}

// Простая функция для чтения ID3v2 тегов из MP3 файла
QString MP3MetadataReader::readID3v2Tag(const QByteArray& data, const QByteArray& frameId) {
    // Ищем ID3v2 заголовок
    if (data.size() < 10 || !data.startsWith("ID3")) {
        return QString();
    }

    // Пропускаем заголовок ID3v2 (10 байт)
    int pos = 10;

    // Читаем размер тега (4 байта, синхросafe integer)
    auto byte6 = static_cast<std::byte>(data[6]);
    auto byte7 = static_cast<std::byte>(data[7]);
    auto byte8 = static_cast<std::byte>(data[8]);
    auto byte9 = static_cast<std::byte>(data[9]);
    const int tagSize = (std::to_integer<std::uint8_t>(byte6) << 21) |
                        (std::to_integer<std::uint8_t>(byte7) << 14) |
                        (std::to_integer<std::uint8_t>(byte8) << 7) |
                        std::to_integer<std::uint8_t>(byte9);

    // Ищем нужный фрейм
    while (pos + 10 < data.size() && pos < 10 + tagSize) {
        QByteArray frameHeader = data.mid(pos, 10);
        QByteArray frameID = frameHeader.left(4);

        // Читаем размер фрейма (4 байта, синхросafe integer)
        auto fbyte4 = static_cast<std::byte>(frameHeader[4]);
        auto fbyte5 = static_cast<std::byte>(frameHeader[5]);
        auto fbyte6 = static_cast<std::byte>(frameHeader[6]);
        auto fbyte7 = static_cast<std::byte>(frameHeader[7]);
        const int frameSize = (std::to_integer<std::uint8_t>(fbyte4) << 21) |
                              (std::to_integer<std::uint8_t>(fbyte5) << 14) |
                              (std::to_integer<std::uint8_t>(fbyte6) << 7) |
                              std::to_integer<std::uint8_t>(fbyte7);

        if (frameID == frameId) {
            // Пропускаем флаги (2 байта)
            QByteArray frameData = data.mid(pos + 10, frameSize);

            // Пропускаем encoding byte (обычно UTF-16 или UTF-8)
            if (frameData.size() <= 1) {
                continue;
            }
            
            auto encodingByte = static_cast<std::byte>(frameData[0]);
            int encoding = std::to_integer<int>(encodingByte);
            
            QString result = processFrameEncoding(frameData, encoding);
            if (!result.isEmpty()) {
                return result.trimmed();
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
        if (auto byte0 = static_cast<std::byte>(buffer[i]);
            std::to_integer<std::uint8_t>(byte0) != 0xFF) {
            continue;
        }
        
        if (auto byte1 = static_cast<std::byte>(buffer[i+1]);
            (std::to_integer<std::uint8_t>(byte1) & 0xE0) != 0xE0) {
            continue;
        }

        // Читаем заголовок MP3 фрейма
        auto header2 = static_cast<std::byte>(buffer[i+1]);
        auto header3 = static_cast<std::byte>(buffer[i+2]);

        // Проверяем версию MP3 (MPEG-1, MPEG-2)
        auto header2Val = std::to_integer<std::uint8_t>(header2);
        auto header3Val = std::to_integer<std::uint8_t>(header3);
        if (auto layer = (header2Val >> 1) & 0x03; layer != 1) {
            continue;
        }

        // Извлекаем битрейт
        auto bitrateIndex = (header3Val >> 4) & 0x0F;
        // Извлекаем частоту дискретизации
        auto sampleRateIndex = (header3Val >> 2) & 0x03;

        // Битрейт для MPEG-1 Layer III (kbps)
        constexpr std::array<int, 16> bitrates = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
        if (bitrateIndex == 0 || bitrateIndex >= 15) {
            continue;
        }
        int bitrate = bitrates[bitrateIndex] * 1000; // в битах в секунду

        // Частота дискретизации для MPEG-1 (Hz)
        constexpr std::array<int, 4> sampleRates = {44100, 48000, 32000, 0};
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

    return 0;
}

namespace {
    struct ID3v1Tags {
        QString title;
        QString artist;
        QString album;
        QString year;
        QString genre;
    };
    
    ID3v1Tags readID3v1Tags(QFile& file) {
        ID3v1Tags tags;
        if (!file.seek(file.size() - 128)) {
            return tags;
        }
        
        QByteArray id3v1 = file.read(128);
        if (!id3v1.startsWith("TAG")) {
            return tags;
        }
        
        tags.title = QString::fromLatin1(id3v1.mid(3, 30)).trimmed();
        tags.artist = QString::fromLatin1(id3v1.mid(33, 30)).trimmed();
        tags.album = QString::fromLatin1(id3v1.mid(63, 30)).trimmed();
        tags.year = QString::fromLatin1(id3v1.mid(93, 4)).trimmed();
        
        auto genreByte = static_cast<std::byte>(id3v1[125]);
        if (int genreIndex = std::to_integer<int>(genreByte); genreIndex < 80) {
            constexpr std::array<const char*, 80> genres = {
                "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk", "Grunge",
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
                "Hard Rock"
            };
            tags.genre = QString::fromUtf8(genres[genreIndex]);
        }
        
        return tags;
    }
    
    struct MetadataFields {
        QString id3Title;
        QString id3Artist;
        QString id3Album;
        QString id3Year;
        QString id3Genre;
    };
    
    struct FillMetadataParams {
        const MetadataFields& metadata;
        QString& title;
        QString& artist;
        QString* album;
        int* year;
        QString* genre;
        int* duration;
        bool& found;
        int calculatedDuration;
    };
    
    void fillMetadataFields(const FillMetadataParams& params) {
        const auto& metadata = params.metadata;
        if (!metadata.id3Title.isEmpty()) {
            params.title = metadata.id3Title;
            params.found = true;
        }
        if (!metadata.id3Artist.isEmpty()) {
            params.artist = metadata.id3Artist;
            params.found = true;
        }
        if (!metadata.id3Album.isEmpty() && params.album != nullptr) {
            *params.album = metadata.id3Album;
        }
        if (!metadata.id3Year.isEmpty() && params.year != nullptr) {
            bool ok;
            int yearValue = metadata.id3Year.left(4).toInt(&ok);
            if (ok && yearValue > 1900 && yearValue < 2100) {
                *params.year = yearValue;
            }
        }
        if (!metadata.id3Genre.isEmpty() && params.genre != nullptr) {
            *params.genre = metadata.id3Genre;
        }
        if (params.duration != nullptr && params.calculatedDuration > 0) {
            *params.duration = params.calculatedDuration;
        }
    }
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
    if (bool needID3v1 = id3Title.isEmpty() || id3Artist.isEmpty() || id3Album.isEmpty() || id3Year.isEmpty();
        needID3v1) {
        file.open(QIODevice::ReadOnly);
        ID3v1Tags id3v1Tags = readID3v1Tags(file);
        file.close();
        
        if (id3Title.isEmpty() && !id3v1Tags.title.isEmpty()) {
            id3Title = id3v1Tags.title;
        }
        if (id3Artist.isEmpty() && !id3v1Tags.artist.isEmpty()) {
            id3Artist = id3v1Tags.artist;
        }
        if (id3Album.isEmpty() && !id3v1Tags.album.isEmpty()) {
            id3Album = id3v1Tags.album;
        }
        if (id3Year.isEmpty() && !id3v1Tags.year.isEmpty()) {
            id3Year = id3v1Tags.year;
        }
        if (id3Genre.isEmpty() && !id3v1Tags.genre.isEmpty()) {
            id3Genre = id3v1Tags.genre;
        }
    }

    // Заполняем поля
    bool found = false;
    int calculatedDuration = 0;
    if (duration != nullptr) {
        calculatedDuration = calculateMP3Duration(filePath);
    }
    
    MetadataFields metadata;
    metadata.id3Title = id3Title;
    metadata.id3Artist = id3Artist;
    metadata.id3Album = id3Album;
    metadata.id3Year = id3Year;
    metadata.id3Genre = id3Genre;
    
    FillMetadataParams fillParams{metadata, title, artist, album, year, genre, duration, found, calculatedDuration};
    fillMetadataFields(fillParams);

    return found;
}

