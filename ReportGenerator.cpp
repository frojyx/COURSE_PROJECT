// ReportGenerator.cpp
#include "ReportGenerator.h"
#include "MusicCatalog.h"
#include "Track.h"

QString ReportGenerator::generateReport(const MusicCatalog& catalog) {
    QString report = "Отчет по музыкальному каталогу\n\n";
    report += "Всего треков: " + QString::number(catalog.getTrackCount()) + "\n\n";

    QList<Track> tracks = catalog.findAllTracks();
    for (const Track& track : tracks) {
        report += QString("ID: %1 | %2 - %3 | %4 | %5\n")
        .arg(track.getId())
            .arg(track.getArtist())
            .arg(track.getTitle())
            .arg(track.getYear())
            .arg(track.getFormattedDuration());
    }

    return report;
}




