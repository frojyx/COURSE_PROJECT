// CSVWriter.h
#ifndef CSVWRITER_H
#define CSVWRITER_H

#include "Track.h"
#include <QString>
#include <QList>

class MusicCatalog;

class CSVWriter {
public:
    static bool saveToCSV(const MusicCatalog& catalog, const QString& filename);
};

#endif // CSVWRITER_H


