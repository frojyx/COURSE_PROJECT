#include "MP3MetadataReader.h"
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QVector>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <bit>

namespace {

// Безопасная конвертация UTF-16
QString convertUTF16Frame(const QByteArray& frameData) {
    if (frameData.size() <= 1) {
        return QString();
    }

    QByteArray utf16Data = frameData.mid(1);
    if (utf16Data.size() % 2 != 0) {
        return QString();
    }

    const int charCount = utf16Data.size() / 2;
    if (charCount <= 0) {
        return QString();
    }

    QVector<char16_t> buffer(charCount);
    std::memcpy(buffer.data(), utf16Data.constData(), utf16Data.size());

    return QString::fromUtf16(buffer.constData(), charCount);
}

// Обработка кодировки ID3 кадра
QString processFrameEncoding(const QByteArray& frameData, int encoding) {
    if (encoding == 0 || encoding == 3) {
        return QString::fromUtf8(frameData.mid(1));
    }
    if (encoding == 1 || encoding == 2) {
        return convertUTF16Frame(frameData);
    }
    return QString();
}

} // namespace

// Чтение ID3v2 тегов (безопасная работа с байтами)
QString MP3MetadataReader::readID3v2Tag(const QByteArray& data, const QByteArray& frameId) {
    if (data.size() < 10 || !data.startsWith("ID3")) {
        return QString();
    }

    auto toByte = [](char c) { return static_cast<std::byte>(static_cast<unsigned char>(c)); };

    std::array<std::byte, 4> sizeBytes = {
        toByte(data[6]),
        toByte(data[7]),
        toByte(data[8]),
        toByte(data[9])
    };

    int tagSize =
        (std::to_integer<int>(sizeBytes[0]) << 21) |
        (std::to_integer<int>(sizeBytes[1]) << 14) |
        (std::to_integer<int>(sizeBytes[2]) << 7) |
        std::to_integer<int>(sizeBytes[3]);

    int pos = 10;
    while (pos + 10 < data.size() && pos < 10 + tagSize) {

        QByteArray header = data.mid(pos, 10);
        QByteArray id = header.left(4);

        std::array<std::byte, 4> frameSizeBytes = {
            toByte(header[4]),
            toByte(header[5]),
            toByte(header[6]),
            toByte(header[7])
        };

        int frameSize =
            (std::to_integer<int>(frameSizeBytes[0]) << 21) |
            (std::to_integer<int>(frameSizeBytes[1]) << 14) |
            (std::to_integer<int>(frameSizeBytes[2]) << 7) |
            std::to_integer<int>(frameSizeBytes[3]);

        if (id == frameId) {
            QByteArray frameData = data.mid(pos + 10, frameSize);
            if (frameData.size() <= 1) {
                continue;
            }

            std::byte encByte = toByte(frameData[0]);
            int encoding = std::to_integer<int>(encByte);

            QString text = processFrameEncoding(frameData, encoding);
            if (!text.isEmpty()) {
                return text.trimmed();
            }
        }

        pos += 10 + frameSize;
    }

    return QString();
}

// Вычисление длительности MP3 (упрощённо)
int MP3MetadataReader::calculateMP3Duration(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return 0;
    }

    qint64 fileSize = QFileInfo(filePath).size();
    QByteArray buffer = file.read(100000);
    file.close();

    auto toByte = [](char c) { return static_cast<std::byte>(static_cast<unsigned char>(c)); };

    for (int i = 0; i < buffer.size() - 4; ++i) {
        std::byte b0 = toByte(buffer[i]);
        std::byte b1 = toByte(buffer[i + 1]);

        if (std::to_integer<unsigned>(b0) != 0xFF) continue;
        if ((std::to_integer<unsigned>(b1) & 0xE0) != 0xE0) continue;

        std::byte h2 = toByte(buffer[i + 1]);
        std::byte h3 = toByte(buffer[i + 2]);

        uint8_t h2val = std::to_integer<uint8_t>(h2);
        uint8_t h3val = std::to_integer<uint8_t>(h3);

        if (((h2val >> 1) & 0x03) != 1) continue;

        int bitrateIndex = (h3val >> 4) & 0x0F;
        int sampleIndex = (h3val >> 2) & 0x03;

        constexpr std::array<int, 16> bitrates = {
            0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0
        };
        if (bitrateIndex == 0 || bitrateIndex >= 15) continue;
        int bitrate = bitrates[bitrateIndex] * 1000;

        constexpr std::array<int,4> sampleRates = {44100,48000,32000,0};
        if (sampleIndex >= 3) continue;
        int sampleRate = sampleRates[sampleIndex];

        if (bitrate > 0 && sampleRate > 0) {
            return (fileSize * 8) / bitrate;
        }
    }

    return 0;
}

// Структуры для ID3v1
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

    QByteArray id = file.read(128);
    if (!id.startsWith("TAG")) {
        return tags;
    }

    tags.title  = QString::fromLatin1(id.mid(3, 30)).trimmed();
    tags.artist = QString::fromLatin1(id.mid(33, 30)).trimmed();
    tags.album  = QString::fromLatin1(id.mid(63, 30)).trimmed();
    tags.year   = QString::fromLatin1(id.mid(93, 4)).trimmed();

    auto toByte = [](char c) { return static_cast<std::byte>(static_cast<unsigned char>(c)); };
    int genreIndex = std::to_integer<int>(toByte(id[125]));

    constexpr std::array<const char*, 80> genres = {
        "Blues","Classic Rock","Country","Dance","Disco","Funk","Grunge","Hip-Hop","Jazz","Metal",
        "New Age","Oldies","Other","Pop","R&B","Rap","Reggae","Rock","Techno","Industrial",
        "Alternative","Ska","Death Metal","Pranks","Soundtrack","Euro-Techno","Ambient","Trip-Hop",
        "Vocal","Jazz+Funk","Fusion","Trance","Classical","Instrumental","Acid","House","Game",
        "Sound Clip","Gospel","Noise","AlternRock","Bass","Soul","Punk","Space","Meditative",
        "Instrumental Pop","Instrumental Rock","Ethnic","Gothic","Darkwave","Techno-Industrial",
        "Electronic","Pop-Folk","Eurodance","Dream","Southern Rock","Comedy","Cult","Gangsta",
        "Top 40","Christian Rap","Pop/Funk","Jungle","Native American","Cabaret","New Wave",
        "Psychadelic","Rave","Showtunes","Trailer","Lo-Fi","Tribal","Acid Punk","Acid Jazz",
        "Polka","Retro","Musical","Rock & Roll","Hard Rock"
    };

    if (genreIndex >= 0 && genreIndex < 80) {
        tags.genre = genres[genreIndex];
    }

    return tags;
}

} // namespace

// Основная функция
bool MP3MetadataReader::readMP3Metadata(const QString& filePath,
                                        QString& title,
                                        QString& artist,
                                        QString* album,
                                        int* year,
                                        QString* genre,
                                        int* duration)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray raw = f.read(65536);
    f.close();

    if (raw.size() < 10) {
        return false;
    }

    QString id3Title  = readID3v2Tag(raw, "TIT2");
    QString id3Artist = readID3v2Tag(raw, "TPE1");
    QString id3Album  = readID3v2Tag(raw, "TALB");
    QString id3Year   = readID3v2Tag(raw, "TDRC");
    QString id3Genre  = readID3v2Tag(raw, "TCON");

    if (id3Year.isEmpty()) {
        id3Year = readID3v2Tag(raw, "TYER");
    }

    if (id3Title.isEmpty() || id3Artist.isEmpty() || id3Album.isEmpty() || id3Year.isEmpty()) {
        f.open(QIODevice::ReadOnly);
        ID3v1Tags v1 = readID3v1Tags(f);
        f.close();

        if (id3Title.isEmpty()  && !v1.title.isEmpty())  id3Title = v1.title;
        if (id3Artist.isEmpty() && !v1.artist.isEmpty()) id3Artist = v1.artist;
        if (id3Album.isEmpty()  && !v1.album.isEmpty())  id3Album = v1.album;
        if (id3Year.isEmpty()   && !v1.year.isEmpty())   id3Year = v1.year;
        if (id3Genre.isEmpty()  && !v1.genre.isEmpty())  id3Genre = v1.genre;
    }

    bool okFound = false;

    if (!id3Title.isEmpty())  { title = id3Title; okFound = true; }
    if (!id3Artist.isEmpty()) { artist = id3Artist; okFound = true; }

    if (album && !id3Album.isEmpty()) {
        *album = id3Album;
    }
    if (genre && !id3Genre.isEmpty()) {
        *genre = id3Genre;
    }

    if (year && !id3Year.isEmpty()) {
        bool ok;
        int y = id3Year.left(4).toInt(&ok);
        if (ok && y > 1900 && y < 2100) {
            *year = y;
        }
    }

    if (duration) {
        int dur = calculateMP3Duration(filePath);
        if (dur > 0) {
            *duration = dur;
        }
    }

    return okFound;
}
