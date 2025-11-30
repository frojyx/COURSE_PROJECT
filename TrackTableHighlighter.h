// TrackTableHighlighter.h
#ifndef TRACKTABLEHIGHLIGHTER_H
#define TRACKTABLEHIGHLIGHTER_H

#include <QTableWidget>
#include <QColor>
#include <QSet>
#include "Track.h"
#include "MusicCatalog.h"

class TrackTableHighlighter {
public:
    explicit TrackTableHighlighter(QTableWidget* table, MusicCatalog* catalog);

    // Подсветка строки таблицы
    void highlightRow(int row, const QColor& color);
    
    // Применение подсветки поиска
    void applySearchHighlighting(const QSet<int>& resultIds);
    
    // Подсветка строки трека из Яндекс Музыки
    void highlightYandexMusicRow(int row, QWidget* actionWidget, const QColor& color);

private:
    QTableWidget* table;
    MusicCatalog* catalog;
};

#endif // TRACKTABLEHIGHLIGHTER_H

