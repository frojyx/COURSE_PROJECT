    // TXTWriter.h
#ifndef TXTWRITER_H
#define TXTWRITER_H

#include "Track.h"
#include <QString>
#include <QList>

class MusicCatalog;

class TXTWriter {
public:
    static bool saveToTXT(const MusicCatalog& catalog, const QString& filename);
};

#endif // TXTWRITER_H

