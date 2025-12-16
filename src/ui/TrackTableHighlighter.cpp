// TrackTableHighlighter.cpp
#include "ui/TrackTableHighlighter.h"
#include <QTableWidgetItem>
#include <QWidget>
#include <QBrush>
#include <QTableWidget>

TrackTableHighlighter::TrackTableHighlighter(QTableWidget* table, MusicCatalog* catalog)
    : table(table), catalog(catalog)
{
}

void TrackTableHighlighter::highlightRow(int row, const QColor& color) const {
    // Подсвечиваем все ячейки со столбцами данных
    for (int col = 0; col < 6; ++col) {
        QTableWidgetItem *cell = table->item(row, col);
        if (cell) {
            if (color.isValid()) {
                cell->setBackground(QBrush(color));
            } else {
                cell->setBackground(QBrush());
            }
        }
    }
    
    // Подсвечиваем виджет с кнопками действий
    QWidget *actionWidget = table->cellWidget(row, 6);
    if (actionWidget) {
        if (color.isValid()) {
            QString colorStyle = QString("background-color: rgb(%1, %2, %3);")
                                   .arg(color.red())
                                   .arg(color.green())
                                   .arg(color.blue());
            actionWidget->setStyleSheet(colorStyle);
        } else {
            actionWidget->setStyleSheet("");
        }
    }
}

void TrackTableHighlighter::applySearchHighlighting(const QSet<int>& resultIds) const {
    QColor highlightColor(255, 255, 180); // мягкая желтая подсветка для поиска
    QColor yandexMusicColor(230, 240, 255); // светло-голубой для Яндекс Музыки
    QColor combinedColor(242, 247, 218); // комбинация желтого и голубого для найденных треков из Яндекс Музыки

    for (int row = 0; row < table->rowCount(); ++row) {
        const QTableWidgetItem *titleItem = table->item(row, 0);
        if (!titleItem) continue;
        int id = titleItem->data(Qt::UserRole).toInt();
        bool isMatch = resultIds.contains(id);

        // Проверяем, является ли трек из Яндекс Музыки
        const Track* track = catalog->findTrackById(id);
        bool isFromYandex = track && track->isFromYandexMusic();

        // Определяем цвет фона в зависимости от статуса
        QColor backgroundColor;
        if (isMatch && isFromYandex) {
            backgroundColor = combinedColor;
        } else if (isMatch) {
            backgroundColor = highlightColor;
        } else if (isFromYandex) {
            backgroundColor = yandexMusicColor;
        } else {
            backgroundColor = QColor(); // прозрачный
        }

        highlightRow(row, backgroundColor);
    }
}

void TrackTableHighlighter::highlightYandexMusicRow(int row, QWidget* actionWidget, const QColor& color) const {
    // Подсвечиваем все ячейки со столбцами данных
    for (int col = 0; col < 6; ++col) {
        QTableWidgetItem *cell = table->item(row, col);
        if (cell) {
            cell->setBackground(QBrush(color));
        }
    }
    
    // Подсвечиваем виджет с кнопками действий
    QString colorStyle = QString("background-color: rgb(%1, %2, %3);")
                           .arg(color.red())
                           .arg(color.green())
                           .arg(color.blue());
    actionWidget->setStyleSheet(colorStyle);
}

