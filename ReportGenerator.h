// ReportGenerator.h
#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <QString>

class MusicCatalog;

class ReportGenerator {
public:
    static QString generateReport(const MusicCatalog& catalog);
};

#endif // REPORTGENERATOR_H


