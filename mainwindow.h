// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QFormLayout>

#include "MusicCatalog.h"
#include "FileManager.h"
#include "MP3FileManager.h"
#include "GenreManager.h"
#include "YandexMusicAPI.h"
#include "YandexMusicIntegrator.h"
#include <QDir>
#include <QDialog>
#include <QListWidget>
#include <QProgressBar>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void showMainCatalog();
    void showAddTrack();
    void addNewTrack();
    void searchTracks();
    void onMP3FileSelected();
    void openTrackFile(int row, int column);
    void autoSaveCatalog();
    void autoLoadCatalog();
    void resetSearch();
    void playTrackById(int trackId);
    void editTrackById(int trackId);
    void deleteTrackById(int trackId);
    void searchYandexMusic();
    void onYandexTracksFound(const QList<YandexTrack>& tracks);
    void onYandexTrackImported(const QString& trackTitle);
    void onYandexError(const QString& errorMessage);
    void importSelectedYandexTracks();

private:
    QStackedWidget *stackedWidget;
    MusicCatalog catalog;
    int currentTrackId;
    MP3FileManager mp3Manager;

    // Экраны
    QWidget *createMainCatalogScreen();
    QWidget *createAddTrackScreen();
    QWidget *createEditTrackScreen();

    // Вспомогательные методы
    void updateTrackTable();
    void updateTrackTable(const QList<Track>& tracksToDisplay);
    void clearAddTrackForm();
    void populateTrackTable(const QList<Track>& tracks);

    // Элементы UI
    QTableWidget *trackTable;
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
    
    // Яндекс Музыка
    YandexMusicIntegrator* yandexIntegrator;
    QDialog* yandexSearchDialog;
    QListWidget* yandexResultsList;
    QLineEdit* yandexSearchEdit;
    QList<YandexTrack> currentYandexTracks;
};

#endif // MAINWINDOW_H
