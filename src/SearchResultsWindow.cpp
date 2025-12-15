// SearchResultsWindow.cpp
#include "SearchResultsWindow.h"
#include <QHeaderView>

SearchResultsWindow::SearchResultsWindow(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

void SearchResultsWindow::setupUI() {
    setWindowTitle("Результаты поиска");
    setMinimumSize(800, 500);

    auto *layout = new QVBoxLayout(this);

    auto *titleLabel = new QLabel("Найденные треки");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");

    resultsTable = new QTableWidget;
    resultsTable->setColumnCount(7);
    resultsTable->setHorizontalHeaderLabels({"ID", "Название", "Исполнитель", "Альбом", "Год", "Жанр", "Длительность"});
    resultsTable->horizontalHeader()->setStretchLastSection(true);
    resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultsTable->setSelectionMode(QAbstractItemView::SingleSelection);

    auto *buttonLayout = new QHBoxLayout;
    auto *closeButton = new QPushButton("Закрыть");
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    layout->addWidget(titleLabel);
    layout->addWidget(resultsTable);
    layout->addLayout(buttonLayout);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(resultsTable, &QTableWidget::cellDoubleClicked, this, &SearchResultsWindow::onTrackDoubleClicked);
}

void SearchResultsWindow::setResults(const QList<Track>& newTracks) {
    tracks = newTracks;
    resultsTable->setRowCount(tracks.size());

    for (int i = 0; i < tracks.size(); ++i) {
        const Track& track = this->tracks[i];
        resultsTable->setItem(i, 0, new QTableWidgetItem(QString::number(track.getId())));
        resultsTable->setItem(i, 1, new QTableWidgetItem(track.getTitle()));
        resultsTable->setItem(i, 2, new QTableWidgetItem(track.getArtist()));
        resultsTable->setItem(i, 3, new QTableWidgetItem(track.getAlbum()));
        resultsTable->setItem(i, 4, new QTableWidgetItem(QString::number(track.getYear())));
        resultsTable->setItem(i, 5, new QTableWidgetItem(track.getGenre()));
        resultsTable->setItem(i, 6, new QTableWidgetItem(track.getFormattedDuration()));
    }
}

void SearchResultsWindow::onTrackDoubleClicked(int row, int column) {
    Q_UNUSED(column)
    if (row >= 0 && row < tracks.size()) {
        emit trackSelected(tracks[row]);
    }
}
