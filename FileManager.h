// FileManager.h
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "CSVWriter.h"
#include "CSVReader.h"
#include "MusicCatalog.h"
#include <QString>

class FileManager {
public:
    static bool saveToCSV(const MusicCatalog& catalog, const QString& filename) {
        return CSVWriter::saveToCSV(catalog, filename);
    }
    static bool loadFromCSV(MusicCatalog& catalog, const QString& filename) {
        return CSVReader::loadFromCSV(catalog, filename);
    }
};

#endif // FILEMANAGER_H
