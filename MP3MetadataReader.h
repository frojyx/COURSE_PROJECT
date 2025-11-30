// MP3MetadataReader.h
#ifndef MP3METADATAREADER_H
#define MP3METADATAREADER_H

#include <QString>

class MP3MetadataReader {
public:
    // Чтение метаданных из MP3 файла (ID3 теги)
    // Параметры album, year, genre, duration опциональны (передавайте nullptr если не нужны)
    static bool readMP3Metadata(const QString& filePath, QString& title, QString& artist,
                                QString* album = nullptr, int* year = nullptr,
                                QString* genre = nullptr, int* duration = nullptr);

private:
    static QString readID3v2Tag(const QByteArray& data, const QByteArray& frameId);
    static int calculateMP3Duration(const QString& filePath);
};

#endif // MP3METADATAREADER_H





