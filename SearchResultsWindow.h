// SearchResultsWindow.h
#ifndef SEARCHRESULTSWINDOW_H
#define SEARCHRESULTSWINDOW_H

#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "Track.h"

class SearchResultsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SearchResultsWindow(QWidget *parent = nullptr);
    void setResults(const QList<Track>& tracks);

signals:
    void trackSelected(const Track& track);

private slots:
    void onTrackDoubleClicked(int row, int column);

private:
    QTableWidget *resultsTable;
    QList<Track> tracks;
    void setupUI();
};

#endif // SEARCHRESULTSWINDOW_H



