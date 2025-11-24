// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QListWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QFormLayout>

#include "MusicCatalog.h"
#include "FileManager.h"
#include "MP3FileManager.h"
#include "SearchResultsWindow.h"
#include "GenreManager.h"
#include "ReportGenerator.h"
#include <QListWidget>
#include <QDir>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void showMainCatalog();
    void showAddTrack();
    void showSearchFilters();
    void showTrackDetails(int row);
    void addNewTrack();
    void searchTracks();
    void onMP3FileSelected();
    void openTrackFile(int row, int column);
    void autoSaveCatalog();
    void autoLoadCatalog();
    void generateReport();
    void deleteSelectedTrack();
    void updateSelectedTrack();
    void saveCatalog();
    void loadCatalog();
    void resetSearch();
    void sortTracks();
    void playTrack(int row);
    void editTrackFromRow(int row);
    void deleteTrackFromRow(int row);
    void playTrackById(int trackId);
    void editTrackById(int trackId);
    void deleteTrackById(int trackId);

private:
    QStackedWidget *stackedWidget;
    MusicCatalog catalog;
    int currentTrackId;
    MP3FileManager mp3Manager;
    SearchResultsWindow *searchResultsWindow;

    // Экраны
    QWidget *createMainCatalogScreen();
    QWidget *createAddTrackScreen();
    QWidget *createSearchScreen();
    QWidget *createTrackDetailsScreen();
    QWidget *createEditTrackScreen();

    // Вспомогательные методы
    void updateTrackTable();
    void updateTrackTable(const QList<Track>& tracksToDisplay);
    void clearAddTrackForm();
    void populateTrackTable(const QList<Track>& tracks);
    Track getSelectedTrack() const;

    // Элементы UI
    QTableWidget *trackTable;
    QComboBox *sortComboBox;
    QLineEdit *searchTitleEdit;
    QLineEdit *searchArtistEdit;
    QLineEdit *searchAlbumEdit;
    QLineEdit *searchGenreEdit;
    QSpinBox *searchMinYear;
    QSpinBox *searchMaxYear;
    QSpinBox *searchMinDuration;
    QSpinBox *searchMaxDuration;

    // Форма добавления трека
    QLabel *selectedFileLabel; // Метка для отображения выбранного файла
    QLineEdit *addTitleEdit;
    QLineEdit *addArtistEdit;
    QLineEdit *addAlbumEdit;
    QSpinBox *addYearEdit;
    QComboBox *addGenreEdit;
    QSpinBox *addDurationEdit;
    QString selectedMP3FilePath;

    // Форма редактирования трека
    QLineEdit *editTitleEdit;
    QLineEdit *editArtistEdit;
    QLineEdit *editAlbumEdit;
    QSpinBox *editYearEdit;
    QComboBox *editGenreEdit;
    QSpinBox *editDurationEdit;

    // Вспомогательные методы для работы с жанрами
    QStringList getGenreList() const;
    void populateGenreComboBox(QComboBox *comboBox);
};

#endif // MAINWINDOW_H
