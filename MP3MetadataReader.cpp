#include "MP3MetadataReader.h"
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QVector>
#include <array>
#include <cstdint>
#include <cstring>

namespace {

// === UTF-16 conversion ===
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

    QVector<char16_t> utf16Buffer(charCount);
    std::memcpy(utf16Buffer.data(), utf16Data.constData(), utf16Data.size());

    return QString::fromUtf16(utf16Buffer.constData(), charCount);
}

// === Frame encoding processor ===
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


// ============================================================================
//                        ID3v2 TAG PARSER
// ============================================================================
QString MP3MetadataReader::readID3v2Tag(const QByteArray& data, const QByteArray& frameId) {

    if (data.size() < 10 || !data.startsWith("ID3")) {
        return QString();
    }

    const uint8_t* d = reinterpret_cast<const uint8_t*>(data.constData());

    // Synchsafe size
    const int tagSize =
        (d[6] << 21) |
        (d[7] << 14) |
        (d[8] << 7)  |
        d[9];

    int pos = 10;

    while (pos + 10 < data.size() && pos < tagSize + 10) {

        QByteArray header = data.mid(pos, 10);
        const uint8_t* h = reinterpret_cast<const uint8_t*>(header.constData());

        QByteArray id = header.left(4);

        const int frameSize =
            (h[4] << 21) |
            (h[5] << 14) |
            (h[6] << 7)  |
            h[7];

        if (id == frameId) {
            QByteArray frameData = data.mid(pos + 10, frameSize);

            if (frameData.size() <= 1) {
                return QString();
            }

            const uint8_t encoding = static_cast<uint8_t>(frameData[0]);

            QString decoded = processFrameEncoding(frameData, encoding);
            if (!decoded.isEmpty()) {
                return decoded.trimmed();
            }
        }

        pos += 10 + frameSize;
    }

    return QString();
}


// ============================================================================
//                        MP3 DURATION SCAN
// ============================================================================
int MP3MetadataReader::calculateMP3Duration(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return 0;
    }

    qint64 fileSize = QFileInfo(filePath).size();
    QByteArray buffer = file.read(100000);
    file.close();

    const uint8_t* b = reinterpret_cast<const uint8_t*>(buffer.constData());

    for (int i = 0; i < buffer.size() - 4; ++i) {

        if (b[i] != 0xFF) {
            continue;
        }

        if ((b[i + 1] & 0xE0) != 0xE0) {
            continue;
        }

        uint8_t header2 = b[i + 1];
        uint8_t header3 = b[i + 2];

        const uint8_t layer = (header2 >> 1) & 0x03;
        if (layer != 1) {
            continue;
        }

        const uint8_t bitrateIndex = (header3 >> 4) & 0x0F;
        const uint8_t sampleRateIndex = (header3 >> 2) & 0x03;

        static constexpr std::array<int, 16> bitratesKbps =
            {0, 32, 40, 48, 56, 64, 80, 96,
             112, 128, 160, 192, 224, 256, 320, 0};

        if (bitrateIndex == 0 || bitrateIndex >= 15) {
            continue;
        }

        int bitrate = bitratesKbps[bitrateIndex] * 1000;

        static constexpr std::array<int, 4> sampleRates =
            {44100, 48000, 32000, 0};

        if (sampleRateIndex >= 3) {
            continue;
        }

        int sampleRate = sampleRates[sampleRateIndex];

        if (bitrate > 0 && sampleRate > 0) {
            return (fileSize * 8) / bitrate;
        }
    }

    return 0;
}


// ============================================================================
//                        ID3v1 TAG SUPPORT
// ============================================================================
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

    QByteArray arr = file.read(128);
    if (!arr.startsWith("TAG")) {
        return tags;
    }

    tags.title  = QString::fromLatin1(arr.mid(3, 30)).trimmed();
    tags.artist = QString::fromLatin1(arr.mid(33, 30)).trimmed();
    tags.album  = QString::fromLatin1(arr.mid(63, 30)).trimmed();
    tags.year   = QString::fromLatin1(arr.mid(93, 4)).trimmed();

    uint8_t genreIndex = static_cast<uint8_t>(arr[125]);
    if (genreIndex < 80) {
        static constexpr std::array<const char*, 80> genres = {
            "Blues","Classic Rock","Country","Dance","Disco","Funk","Grunge",
            "Hip-Hop","Jazz","Metal","New Age","Oldies","Other","Pop","R&B",
            "Rap","Reggae","Rock","Techno","Industrial","Alternative","Ska",
            "Death Metal","Pranks","Soundtrack","Euro-Techno","Ambient",
            "Trip-Hop","Vocal","Jazz+Funk","Fusion","Trance","Classical",
            "Instrumental","Acid","House","Game","Sound Clip","Gospel","Noise",
            "AlternRock","Bass","Soul","Punk","Space","Meditative",
            "Instrumental Pop","Instrumental Rock","Ethnic","Gothic","Darkwave",
            "Techno-Industrial","Electronic","Pop-Folk","Eurodance","Dream",
            "Southern Rock","Comedy","Cult","Gangsta","Top 40","Christian Rap",
            "Pop/Funk","Jungle","Native American","Cabaret","New Wave",
            "Psychadelic","Rave","Showtunes","Trailer","Lo-Fi","Tribal",
            "Acid Punk","Acid Jazz","Polka","Retro","Musical","Rock & Roll",
            "Hard Rock"
        };
        tags.genre = QString::fromUtf8(genres[genreIndex]);
    }

    return tags;
}


// ============================================================================
//                       Metadata filler
// ============================================================================
struct MetadataFields {
    QString title;
    QString artist;
    QString album;
    QString year;
    QString genre;
};

void fillMetadata(
    const MetadataFields& src,
    QString& title,
    QString& artist,
    QString* album,
    int* year,
    QString* genre,
    int durationValue,
    int* durationOut,
    bool& found
    ) {
    if (!src.title.isEmpty()) {
        title = src.title;
        found = true;
    }
    if (!src.artist.isEmpty()) {
        artist = src.artist;
        found = true;
    }
    if (album && !src.album.isEmpty()) {
        *album = src.album;
    }
    if (year && !src.year.isEmpty()) {
        bool ok;
        int y = src.year.left(4).toInt(&ok);
        if (ok && y > 1900 && y < 2100) {
            *year = y;
        }
    }
    if (genre && !src.genre.isEmpty()) {
        *genre = src.genre;
    }
    if (durationOut && durationValue > 0) {
        *durationOut = durationValue;
    }
}

} // namespace


// ============================================================================
//                      MAIN ENTRY POINT
// ============================================================================
bool MP3MetadataReader::readMP3Metadata(
    const QString& filePath,
    QString& title,
    QString& artist,
    QString* album,
    int* year,
    QString* genre,
    int* duration
    ) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.read(65536);
    file.close();

    if (data.size() < 10) {
        return false;
    }

    // ID3v2
    QString t = readID3v2Tag(data, "TIT2");
    QString a = readID3v2Tag(data, "TPE1");
    QString alb = readID3v2Tag(data, "TALB");
    QString yr  = readID3v2Tag(data, "TDRC");

    if (yr.isEmpty()) {
        yr = readID3v2Tag(data, "TYER");
    }

    QString gn = readID3v2Tag(data, "TCON");

    // ID3v1 fallback
    if (t.isEmpty() || a.isEmpty() || alb.isEmpty() || yr.isEmpty()) {

        file.open(QIODevice::ReadOnly);
        ID3v1Tags legacy = readID3v1Tags(file);
        file.close();

        if (t.isEmpty() && !legacy.title.isEmpty()) t = legacy.title;
        if (a.isEmpty() && !legacy.artist.isEmpty()) a = legacy.artist;
        if (alb.isEmpty() && !legacy.album.isEmpty()) alb = legacy.album;
        if (yr.isEmpty() && !legacy.year.isEmpty()) yr = legacy.year;
        if (gn.isEmpty() && !legacy.genre.isEmpty()) gn = legacy.genre;
    }

    const int durationValue =
        duration ? calculateMP3Duration(filePath) : 0;

    MetadataFields m {t, a, alb, yr, gn};

    bool found = false;
    fillMetadata(m, title, artist, album, year, genre, durationValue, duration, found);

    return found;
}
