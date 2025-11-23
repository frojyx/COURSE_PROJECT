// FileManager.h
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "TXTWriter.h"
#include "TXTReader.h"
#include "MusicCatalog.h"
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
