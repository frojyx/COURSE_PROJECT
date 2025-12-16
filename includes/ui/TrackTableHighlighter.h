// TrackTableHighlighter.h
#ifndef TRACKTABLEHIGHLIGHTER_H
#define TRACKTABLEHIGHLIGHTER_H

#include <QTableWidget>
#include <QColor>
#include <QSet>
#include "core/Track.h"
#include "core/MusicCatalog.h"

class TrackTableHighlighter {
public:
    explicit TrackTableHighlighter(QTableWidget* table, MusicCatalog* catalog);

    // Подсветка строки таблицы
    void highlightRow(int row, const QColor& color) const;
    
    // Применение подсветки поиска
    void applySearchHighlighting(const QSet<int>& resultIds) const;
    
    // Подсветка строки трека из Яндекс Музыки
    void highlightYandexMusicRow(int row, QWidget* actionWidget, const QColor& color) const;

private:
    QTableWidget* table;
    MusicCatalog* catalog;
};

#endif // TRACKTABLEHIGHLIGHTER_H

