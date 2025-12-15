// MP3FileManager.cpp
#include "MP3FileManager.h"

MP3FileManager::MP3FileManager(const QString& musicPath)
    : fileFinder(musicPath)
{
}

QList<QFileInfo> MP3FileManager::getMP3Files() const {
    return fileFinder.getMP3Files();
}

bool MP3FileManager::parseFileName(const QString& fileName, QString& title, QString& artist,
                                   QString* album, int* year, QString* genre, int* duration) {
    return MP3FileNameParser::parseFileName(fileName, title, artist, album, year, genre, duration);
}

bool MP3FileManager::readMP3Metadata(const QString& filePath, QString& title, QString& artist,
                                     QString* album, int* year, QString* genre, int* duration) {
    return MP3MetadataReader::readMP3Metadata(filePath, title, artist, album, year, genre, duration);
}

QString MP3FileManager::createNewFileName(int id, const QString& title, const QString& artist,
                                          const QString& album, int year, const QString& genre, int duration) {
    return MP3FileOperations::createNewFileName(id, title, artist, album, year, genre, duration);
}

bool MP3FileManager::renameFile(const QString& oldPath, const QString& newFileName) {
    return MP3FileOperations::renameFile(oldPath, newFileName);
}

bool MP3FileManager::openMP3File(const QString& filePath) {
    return MP3FileOperations::openMP3File(filePath);
}

QString MP3FileManager::findFileByTrack(int id, const QString& title, const QString& artist,
                                        const QString& album, int year, const QString& genre, int duration) const {
    return fileFinder.findFileByTrack(id, title, artist, album, year, genre, duration);
}

QString MP3FileManager::getMusicPath() const {
    return fileFinder.getMusicPath();
}
