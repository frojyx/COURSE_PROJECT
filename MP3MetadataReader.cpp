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
    if (frameData.size() <= 1) return {};

    QByteArray utf16Data = frameData.mid(1);
    if (utf16Data.size() % 2 != 0) return {};

    const int charCount = utf16Data.size() / 2;
    if (charCount <= 0) return {};

    QVector<char16_t> buffer(charCount);
    std::memcpy(buffer.data(), utf16Data.constData(), utf16Data.size());
    return QString::fromUtf16(buffer.constData(), charCount);
}

// Унифицированная обработка кодировки ID3
QString processFrameEncoding(const QByteArray& frameData, int encoding) {
    switch (encoding) {
    case 0:
    case 3:
        return QString::fromUtf8(frameData.mid(1));
    case 1:
    case 2:
        return convertUTF16Frame(frameData);
    default:
        return {};
    }
}

// Считывание числа из ID3 синхронного формата
int readSynchsafeInt(std::array<std::byte,4> b) {
    return (std::to_integer<int>(b[0]) << 21) |
           (std::to_integer<int>(b[1]) << 14) |
           (std::to_integer<int>(b[2]) << 7)  |
           std::to_integer<int>(b[3]);
}

} // namespace

// Чтение ID3v2 тегов
QString MP3MetadataReader::readID3v2Tag(const QByteArray& data, const QByteArray& frameId) {
    if (data.size() < 10 || !data.startsWith("ID3")) return {};

    auto toByte = [](char c) { return std::byte(static_cast<unsigned char>(c)); };

    std::array<std::byte,4> sizeBytes = {
        toByte(data[6]), toByte(data[7]),
        toByte(data[8]), toByte(data[9])
    };

    const int tagSize = readSynchsafeInt(sizeBytes);

    int pos = 10;
    while (pos + 10 < data.size() && pos < 10 + tagSize) {

        auto header = data.mid(pos, 10);
        auto id = header.left(4);

        std::array<std::byte,4> frameSizeBytes = {
            toByte(header[4]), toByte(header[5]),
            toByte(header[6]), toByte(header[7])
        };

        const int frameSize = readSynchsafeInt(frameSizeBytes);
        if (frameSize <= 0) break;

        if (id == frameId) {
            auto frame = data.mid(pos + 10, frameSize);
            if (frame.size() > 1) {
                int enc = std::to_integer<int>(toByte(frame[0]));
                QString text = processFrameEncoding(frame, enc);
                if (!text.isEmpty()) return text.trimmed();
            }
        }

        pos += 10 + frameSize;
    }

    return {};
}

// Вычисление длительности MP3
int MP3MetadataReader::calculateMP3Duration(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return 0;

    const auto fileSize = QFileInfo(filePath).size();
    auto buffer = file.read(100000);
    file.close();

    for (int i = 0; i < buffer.size() - 4; ++i) {

        auto b0 = std::byte(static_cast<unsigned char>(buffer[i]));
        auto b1 = std::byte(static_cast<unsigned char>(buffer[i + 1]));
        auto b2 = std::byte(static_cast<unsigned char>(buffer[i + 2]));

        if (std::to_integer<unsigned>(b0) != 0xFF) continue;
        if ((std::to_integer<unsigned>(b1) & 0xE0) != 0xE0) continue;

        const auto h2 = std::to_integer<uint8_t>(b1);
        const auto h3 = std::to_integer<uint8_t>(b2);

        if (((h2 >> 1) & 0x03) != 1) continue;

        const int bitrateIndex = (h3 >> 4) & 0x0F;
        const int sampleIndex = (h3 >> 2) & 0x03;

        constexpr std::array<int, 16> bitrates = {
            0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0
        };
        if (bitrateIndex == 0 || bitrateIndex >= 15) continue;
        const int bitrate = bitrates[bitrateIndex] * 1000;

        constexpr std::array<int, 4> sampleRates = {44100,48000,32000,0};
        if (sampleIndex >= 3) continue;
        const int sampleRate = sampleRates[sampleIndex];

        if (bitrate > 0 && sampleRate > 0)
            return static_cast<int>((fileSize * 8) / bitrate);
    }

    return 0;
}

namespace {

struct ID3v1Tags {
    QString title, artist, album, year, genre;
};

ID3v1Tags readID3v1Tags(QFile& file) {
    ID3v1Tags tags;

    if (!file.seek(file.size() - 128)) return tags;

    auto id = file.read(128);
    if (!id.startsWith("TAG")) return tags;

    tags.title  = QString::fromLatin1(id.mid(3, 30)).trimmed();
    tags.artist = QString::fromLatin1(id.mid(33, 30)).trimmed();
    tags.album  = QString::fromLatin1(id.mid(63, 30)).trimmed();
    tags.year   = QString::fromLatin1(id.mid(93, 4)).trimmed();

    int genreIndex = static_cast<unsigned char>(id[125]);

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

    if (genreIndex >= 0 && genreIndex < 80)
        tags.genre = genres[genreIndex];

    return tags;
}

} // namespace

// Основная функция
bool MP3MetadataReader::readMP3Metadata(
    const QString& filePath,
    QString& title,
    QString& artist,
    QString* album,
    int* year,
    QString* genre,
    int* duration)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) return false;

    auto raw = f.read(65536);
    f.close();

    if (raw.size() < 10) return false;

    auto id3Title  = readID3v2Tag(raw, "TIT2");
    auto id3Artist = readID3v2Tag(raw, "TPE1");
    auto id3Album  = readID3v2Tag(raw, "TALB");
    auto id3Year   = readID3v2Tag(raw, "TDRC");
    auto id3Genre  = readID3v2Tag(raw, "TCON");

    if (id3Year.isEmpty())
        id3Year = readID3v2Tag(raw, "TYER");

    if (id3Title.isEmpty() ||
        id3Artist.isEmpty() ||
        id3Album.isEmpty() ||
        id3Year.isEmpty())
    {
        f.open(QIODevice::ReadOnly);
        auto v1 = readID3v1Tags(f);
        f.close();

        if (id3Title.isEmpty()  && !v1.title.isEmpty())  id3Title = v1.title;
        if (id3Artist.isEmpty() && !v1.artist.isEmpty()) id3Artist = v1.artist;
        if (id3Album.isEmpty()  && !v1.album.isEmpty())  id3Album = v1.album;
        if (id3Year.isEmpty()   && !v1.year.isEmpty())   id3Year = v1.year;
        if (id3Genre.isEmpty()  && !v1.genre.isEmpty())  id3Genre = v1.genre;
    }

    bool ok = false;

    if (!id3Title.isEmpty())  { title = id3Title; ok = true; }
    if (!id3Artist.isEmpty()) { artist = id3Artist; ok = true; }

    if (album && !id3Album.isEmpty())  *album = id3Album;
    if (genre && !id3Genre.isEmpty())  *genre = id3Genre;

    if (year && !id3Year.isEmpty()) {
        bool okYear;
        const int y = id3Year.left(4).toInt(&okYear);
        if (okYear && y > 1900 && y < 2100) *year = y;
    }

    if (duration) {
        const int dur = calculateMP3Duration(filePath);
        if (dur > 0) *duration = dur;
    }

    return ok;
}
