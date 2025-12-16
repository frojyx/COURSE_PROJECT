// FileManager.h
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "file_operations/TXTWriter.h"
#include "file_operations/TXTReader.h"
#include "core/MusicCatalog.h"
#include <QString>

class FileManager {
public:
    static bool saveToTXT(const MusicCatalog& catalog, const QString& filename) {
        return TXTWriter::saveToTXT(catalog, filename);
    }
    static bool loadFromTXT(MusicCatalog& catalog, const QString& filename) {
        return TXTReader::loadFromTXT(catalog, filename);
    }
};

#endif // FILEMANAGER_H
