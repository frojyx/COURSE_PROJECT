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
#include "TrackSearchParams.h"
#include <QDir>
#include <QDialog>
#include <QListWidget>
#include <QProgressBar>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void showMainCatalog();
    void showAddTrack();
    void addNewTrack();
    void searchTracks();
    void onMP3FileSelected();
    void openTrackFile(int row, int column);
    void autoSaveCatalog() const;
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
    // Основные компоненты
    QStackedWidget *stackedWidget;
    MusicCatalog catalog;
    int currentTrackId = -1;
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
    void fillFormFromParsedFileName(const QString& fileBaseName, const QString& title, 
                                    const QString& artist, const QString& parsedAlbum,
                                    int parsedYear, const QString& parsedGenre, int parsedDuration);
    void fillFormFromMP3Metadata(const QString& fileName);

    // Структуры для группировки UI элементов
    struct SearchUI {
        QTableWidget *trackTable = nullptr;
        QLineEdit *searchTitleEdit = nullptr;
        QLineEdit *searchArtistEdit = nullptr;
        QLineEdit *searchAlbumEdit = nullptr;
        QLineEdit *searchGenreEdit = nullptr;
        QSpinBox *searchMinYear = nullptr;
        QSpinBox *searchMaxYear = nullptr;
        QSpinBox *searchMinDuration = nullptr;
        QSpinBox *searchMaxDuration = nullptr;
    } searchUI;

    struct AddTrackUI {
        QLabel *selectedFileLabel = nullptr;
        QLineEdit *addTitleEdit = nullptr;
        QLineEdit *addArtistEdit = nullptr;
        QLineEdit *addAlbumEdit = nullptr;
        QSpinBox *addYearEdit = nullptr;
        QComboBox *addGenreEdit = nullptr;
        QSpinBox *addDurationEdit = nullptr;
        QString selectedMP3FilePath;
    } addTrackUI;

    struct EditTrackUI {
        QLineEdit *editTitleEdit = nullptr;
        QLineEdit *editArtistEdit = nullptr;
        QLineEdit *editAlbumEdit = nullptr;
        QSpinBox *editYearEdit = nullptr;
        QComboBox *editGenreEdit = nullptr;
        QSpinBox *editDurationEdit = nullptr;
    } editTrackUI;

    struct YandexMusicUI {
        YandexMusicIntegrator* integrator = nullptr;
        QDialog* searchDialog = nullptr;
        QListWidget* resultsList = nullptr;
        QLineEdit* searchEdit = nullptr;
        QList<YandexTrack> currentTracks;
    } yandexUI;

    // Вспомогательные методы для работы с жанрами
    QStringList getGenreList() const;
    void populateGenreComboBox(QComboBox *comboBox) const;
};

#endif // MAINWINDOW_H
