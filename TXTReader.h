// TXTReader.h
#ifndef TXTREADER_H
#define TXTREADER_H

#include <QString>

class MusicCatalog;

class TXTReader {
public:
    static bool loadFromTXT(MusicCatalog& catalog, const QString& filename);
};

#endif // TXTREADER_H

