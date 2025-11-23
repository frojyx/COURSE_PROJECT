// CSVReader.h
#ifndef CSVREADER_H
#define CSVREADER_H

#include <QString>

class MusicCatalog;

class CSVReader {
public:
    static bool loadFromCSV(MusicCatalog& catalog, const QString& filename);
};

#endif // CSVREADER_H


