// mainwindow.cpp
#include "MainWindow.h"
#include <QFormLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QGroupBox>
#include <QSpinBox>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QColor>
#include <QSet>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentTrackId(-1), yandexIntegrator(nullptr), yandexSearchDialog(nullptr)
{
    setWindowTitle("Музыкальный каталог");
    setMinimumSize(1000, 700);

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    // Инициализируем интегратор Яндекс Музыки
    yandexIntegrator = new YandexMusicIntegrator(&catalog, this);
    connect(yandexIntegrator, &YandexMusicIntegrator::tracksFound,
            this, &MainWindow::onYandexTracksFound);
    connect(yandexIntegrator, &YandexMusicIntegrator::trackImported,
            this, &MainWindow::onYandexTrackImported);
    connect(yandexIntegrator, &YandexMusicIntegrator::errorOccurred,
            this, &MainWindow::onYandexError);

    // Создаем экраны
    stackedWidget->addWidget(createMainCatalogScreen());
    stackedWidget->addWidget(createAddTrackScreen());
    stackedWidget->addWidget(createEditTrackScreen());

    // Автозагрузка каталога при запуске
    autoLoadCatalog();

    // Показываем главный экран
    showMainCatalog();
}

void MainWindow::showMainCatalog() {
    updateTrackTable();
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::showAddTrack() {
    clearAddTrackForm();
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::addNewTrack() {
    if (selectedMP3FilePath.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите MP3 файл");
        return;
    }

    if (addTitleEdit->text().isEmpty() || addArtistEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните обязательные поля (Название и Исполнитель)");
        return;
    }

    // Добавляем трек в каталог
    int newId = catalog.getNextId();

    // Переименовываем файл
    QString newFileName = MP3FileManager::createNewFileName(newId,
                                                            addTitleEdit->text(),
                                                            addArtistEdit->text(),
                                                            addAlbumEdit->text(),
                                                            addYearEdit->value(),
                                                            addGenreEdit->currentText(),
                                                            addDurationEdit->value());

    QString newFilePath = QFileInfo(selectedMP3FilePath).absolutePath() + "/" + newFileName;

    if (!MP3FileManager::renameFile(selectedMP3FilePath, newFileName)) {
        QMessageBox::warning(this, "Предупреждение", "Не удалось переименовать файл");
        // Используем старый путь
        newFilePath = selectedMP3FilePath;
    }

    catalog.addTrack(addTitleEdit->text(),
                     addArtistEdit->text(),
                     addAlbumEdit->text(),
                     addYearEdit->value(),
                     addGenreEdit->currentText(),
                     addDurationEdit->value(),
                     newFilePath);

    // Автосохранение
    autoSaveCatalog();

    QMessageBox::information(this, "Успех", "Трек успешно добавлен!");

    showMainCatalog();
}

void MainWindow::searchTracks() {
    QString title = searchTitleEdit->text();
    QString artist = searchArtistEdit->text();
    QString album = searchAlbumEdit->text();
    QString genre = searchGenreEdit->text();

    int minYear = searchMinYear->value();
    int maxYear = searchMaxYear->value();
    int minDuration = searchMinDuration->value();
    int maxDuration = searchMaxDuration->value();

    // Получаем результаты фильтрации
    TrackSearchParams params;
    params.title = title;
    params.artist = artist;
    params.album = album;
    params.genre = genre;
    params.minYear = minYear;
    params.maxYear = maxYear;
    params.minDuration = minDuration;
    params.maxDuration = maxDuration;
    
    QList<Track> results = catalog.searchTracksWithFilters(params);

    // Подсветка строк в текущей таблице
    updateTrackTable(); // гарантируем, что таблица содержит все треки

    // Сформируем множество id найденных треков
    QSet<int> resultIds;
    for (const Track& t : results) {
        resultIds.insert(t.getId());
    }

    QColor highlightColor(255, 255, 180); // мягкая желтая подсветка

    for (int row = 0; row < trackTable->rowCount(); ++row) {
        QTableWidgetItem *titleItem = trackTable->item(row, 0);
        if (!titleItem) continue;
        int id = titleItem->data(Qt::UserRole).toInt();
        bool isMatch = resultIds.contains(id);

        // Применяем фон ко всем ячейкам строки, кроме виджета действий
        for (int col = 0; col < 6; ++col) {
            QTableWidgetItem *cell = trackTable->item(row, col);
            if (!cell) continue;
            cell->setBackground(isMatch ? QBrush(highlightColor) : QBrush());
        }
    }

    if (results.isEmpty()) {
        QMessageBox::information(this, "Поиск", "Треки не найдены");
    }
}


void MainWindow::autoSaveCatalog() {
    QString fileName = "catalog_autosave.txt";
    FileManager::saveToTXT(catalog, fileName);
}

void MainWindow::autoLoadCatalog() {
    QString fileName = "catalog_autosave.txt";
    if (QFile::exists(fileName)) {
        FileManager::loadFromTXT(catalog, fileName);
    }
}

void MainWindow::onMP3FileSelected() {
    // Открываем диалог выбора файла
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Выберите MP3 файл",
                                                    "D:\\music",
                                                    "MP3 Files (*.mp3)");

    if (fileName.isEmpty()) {
        selectedMP3FilePath.clear();
        if (selectedFileLabel) {
            selectedFileLabel->setText("Файл не выбран");
            selectedFileLabel->setStyleSheet("color: gray; margin: 5px;");
        }
        return;
    }

    selectedMP3FilePath = fileName;
    QString fileBaseName = QFileInfo(fileName).fileName();

    // Обновляем метку выбранного файла
    if (selectedFileLabel) {
        selectedFileLabel->setText("Выбран: " + fileBaseName);
        selectedFileLabel->setStyleSheet("color: green; margin: 5px;");
    }

    // Переменные для всех полей
    QString title, artist;

    // Сначала пытаемся парсить имя файла (полный формат: ID.название.исполнитель.альбом.год.жанр.длительность.mp3)
    QString parsedAlbum, parsedGenre;
    int parsedYear = 2024;
    int parsedDuration = 180;

    if (MP3FileManager::parseFileName(fileBaseName, title, artist, &parsedAlbum, &parsedYear, &parsedGenre, &parsedDuration)) {
        // Если удалось распарсить имя файла
        addTitleEdit->setText(title);
        addArtistEdit->setText(artist);

        // Проверяем, есть ли полный формат (7 частей: ID.название.исполнитель.альбом.год.жанр.длительность)
        QString baseName = QFileInfo(fileBaseName).baseName();
        if (!baseName.isEmpty() && baseName[0].isDigit()) {
            QStringList parts = baseName.split('.');
            if (parts.size() >= 7) {
                // Полный формат найден - заполняем все поля
                if (!parsedAlbum.isEmpty()) {
                    addAlbumEdit->setText(parsedAlbum);
                }
                if (parsedYear > 1900 && parsedYear < 2100) {
                    addYearEdit->setValue(parsedYear);
                }
                if (!parsedGenre.isEmpty()) {
                    // Пытаемся найти жанр в списке
                    int index = addGenreEdit->findText(parsedGenre, Qt::MatchExactly);
                    if (index >= 0) {
                        addGenreEdit->setCurrentIndex(index);
                    } else {
                        // Если жанр не найден в списке, оставляем пустой элемент
                        addGenreEdit->setCurrentIndex(0);
                    }
                }
                if (parsedDuration > 0) {
                    addDurationEdit->setValue(parsedDuration);
                }
            } else {
                // Неполный формат - заполняем только название и исполнителя, остальное ждем ввода
                // Альбом, год, жанр, длительность остаются для ручного ввода
            }
        } else {
            // Простой формат (название.исполнитель) - заполняем только название и исполнителя
            // Альбом, год, жанр, длительность остаются для ручного ввода
        }
    } else {
        // Если не удалось распарсить имя файла, пытаемся прочитать метаданные из MP3
        // Но заполняем ТОЛЬКО название и исполнителя из метаданных
        QString titleFromMeta, artistFromMeta;
        if (MP3FileManager::readMP3Metadata(fileName, titleFromMeta, artistFromMeta)) {
            addTitleEdit->setText(titleFromMeta);
            addArtistEdit->setText(artistFromMeta);
            // Альбом, год, жанр, длительность НЕ заполняем - остаются для ручного ввода
        }
    }
}

void MainWindow::openTrackFile(int row, int column) {
    Q_UNUSED(column);
    if (row < 0) return;

    QTableWidgetItem *item = trackTable->item(row, 0);
    if (!item) return;
    int trackId = item->data(Qt::UserRole).toInt();
    Track* track = catalog.findTrackById(trackId);

    if (!track) return;
    
    QString filePath = track->getFilePath();
    
    // Проверяем, является ли путь ссылкой на Яндекс Музыку
    if (!filePath.isEmpty() && (filePath.startsWith("https://music.yandex.ru/") || 
                                filePath.startsWith("http://music.yandex.ru/"))) {
        // Открываем ссылку в браузере
        QDesktopServices::openUrl(QUrl(filePath));
        return;
    }
    
    // Обычный локальный файл
    if (!filePath.isEmpty()) {
        MP3FileManager::openMP3File(filePath);
    } else {
        // Если путь не сохранен, пытаемся найти файл
        QString foundPath = mp3Manager.findFileByTrack(track->getId(),
                                                       track->getTitle(),
                                                       track->getArtist(),
                                                       track->getAlbum(),
                                                       track->getYear(),
                                                       track->getGenre(),
                                                       track->getDuration());
        if (!foundPath.isEmpty()) {
            MP3FileManager::openMP3File(foundPath);
        } else {
            QMessageBox::warning(this, "Ошибка", "Файл не найден");
        }
    }
}


void MainWindow::playTrackById(int trackId) {
    Track* track = catalog.findTrackById(trackId);

    if (!track) {
        QMessageBox::warning(this, "Ошибка", "Трек не найден");
        return;
    }

    QString filePath = track->getFilePath();
    
    // Проверяем, является ли путь ссылкой на Яндекс Музыку
    if (!filePath.isEmpty() && (filePath.startsWith("https://music.yandex.ru/") || 
                                filePath.startsWith("http://music.yandex.ru/"))) {
        // Открываем ссылку в браузере
        QDesktopServices::openUrl(QUrl(filePath));
        return;
    }
    
    // Обычный локальный файл
    if (!filePath.isEmpty()) {
        MP3FileManager::openMP3File(filePath);
    } else {
        // Если путь не сохранен, пытаемся найти файл
        QString foundPath = mp3Manager.findFileByTrack(track->getId(),
                                                       track->getTitle(),
                                                       track->getArtist(),
                                                       track->getAlbum(),
                                                       track->getYear(),
                                                       track->getGenre(),
                                                       track->getDuration());
        if (!foundPath.isEmpty()) {
            MP3FileManager::openMP3File(foundPath);
        } else {
            QMessageBox::warning(this, "Ошибка", "Файл не найден");
        }
    }
}

void MainWindow::editTrackById(int trackId) {
    Track* track = catalog.findTrackById(trackId);

    if (!track) {
        QMessageBox::warning(this, "Ошибка", "Трек не найден");
        return;
    }

    // Заполняем форму редактирования данными трека
    editTitleEdit->setText(track->getTitle());
    editArtistEdit->setText(track->getArtist());
    editAlbumEdit->setText(track->getAlbum());
    editYearEdit->setValue(track->getYear());
    // Пытаемся найти жанр в списке
    int index = editGenreEdit->findText(track->getGenre(), Qt::MatchExactly);
    if (index >= 0) {
        editGenreEdit->setCurrentIndex(index);
    } else {
        // Если жанр не найден в списке, устанавливаем текст (для редактируемого комбобокса)
        editGenreEdit->setCurrentText(track->getGenre());
    }
    editDurationEdit->setValue(track->getDuration());

    currentTrackId = trackId;
    stackedWidget->setCurrentIndex(2); // Экран редактирования
}

void MainWindow::deleteTrackById(int trackId) {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение",
                                                              "Вы уверены, что хотите удалить этот трек?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (catalog.removeTrack(trackId)) {
            QMessageBox::information(this, "Успех", "Трек удален");
            updateTrackTable();
            autoSaveCatalog();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось удалить трек");
        }
    }
}

void MainWindow::resetSearch() {
    searchTitleEdit->clear();
    searchArtistEdit->clear();
    searchAlbumEdit->clear();
    searchGenreEdit->clear();
    searchMinYear->setValue(1900);
    searchMaxYear->setValue(2100);
    searchMinDuration->setValue(1);
    searchMaxDuration->setValue(3600);
}


void MainWindow::updateTrackTable() {
    QList<Track> tracks = catalog.findAllTracks();
    populateTrackTable(tracks);
}

void MainWindow::updateTrackTable(const QList<Track>& tracksToDisplay) {
    populateTrackTable(tracksToDisplay);
}

void MainWindow::populateTrackTable(const QList<Track>& tracks) {
    // Временно отключаем сортировку при заполнении таблицы
    bool sortingWasEnabled = trackTable->isSortingEnabled();
    trackTable->setSortingEnabled(false);

    trackTable->setRowCount(tracks.size());

    for (int i = 0; i < tracks.size(); ++i) {
        const Track& track = tracks[i];

        // Сохраняем trackId в данных первого столбца (название)
        QTableWidgetItem *titleItem = new QTableWidgetItem(track.getTitle());
        titleItem->setData(Qt::UserRole, track.getId());
        trackTable->setItem(i, 0, titleItem);

        trackTable->setItem(i, 1, new QTableWidgetItem(track.getArtist()));
        trackTable->setItem(i, 2, new QTableWidgetItem(track.getAlbum()));
        trackTable->setItem(i, 3, new QTableWidgetItem(QString::number(track.getYear())));
        trackTable->setItem(i, 4, new QTableWidgetItem(track.getGenre()));
        trackTable->setItem(i, 5, new QTableWidgetItem(track.getFormattedDuration()));

        // Создаем виджет с кнопками для столбца действий
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(3, 2, 3, 2);
        actionLayout->setSpacing(5);

        QPushButton *playButton = new QPushButton("▶");
        playButton->setToolTip("Проиграть");
        playButton->setFixedSize(28, 24);
        playButton->setStyleSheet("QPushButton { font-size: 12px; }");

        QPushButton *editButton = new QPushButton("✎");
        editButton->setToolTip("Редактировать");
        editButton->setFixedSize(28, 24);
        editButton->setStyleSheet("QPushButton { font-size: 12px; }");

        QPushButton *deleteButton = new QPushButton("✗");
        deleteButton->setToolTip("Удалить");
        deleteButton->setFixedSize(28, 24);
        deleteButton->setStyleSheet("QPushButton { font-size: 12px; color: red; }");

        actionLayout->addWidget(playButton);
        actionLayout->addWidget(editButton);
        actionLayout->addWidget(deleteButton);
        actionLayout->addStretch();
        actionLayout->setAlignment(Qt::AlignCenter);

        actionWidget->setLayout(actionLayout);
        trackTable->setCellWidget(i, 6, actionWidget);

        // Сохраняем trackId для использования в лямбдах (чтобы избежать проблем при сортировке)
        int trackId = track.getId();

        // Подключаем сигналы кнопок (используем trackId для надежности при сортировке)
        connect(playButton, &QPushButton::clicked, [this, trackId]() {
            this->playTrackById(trackId);
        });

        connect(editButton, &QPushButton::clicked, [this, trackId]() {
            this->editTrackById(trackId);
        });

        connect(deleteButton, &QPushButton::clicked, [this, trackId]() {
            this->deleteTrackById(trackId);
        });
    }

    // Включаем сортировку обратно, если она была включена
    trackTable->setSortingEnabled(sortingWasEnabled);
}

QStringList MainWindow::getGenreList() const {
    return GenreManager::getGenreList();
}

void MainWindow::populateGenreComboBox(QComboBox *comboBox) {
    if (!comboBox) return;
    comboBox->clear();
    comboBox->addItem(""); // Пустой элемент для сброса
    comboBox->addItems(getGenreList());
    comboBox->setEditable(false); // Только выбор из списка, без возможности ввода вручную
}

void MainWindow::clearAddTrackForm() {
    addTitleEdit->clear();
    addArtistEdit->clear();
    addAlbumEdit->clear();
    addYearEdit->setValue(2024);
    addGenreEdit->setCurrentIndex(0); // Сбрасываем на первый элемент
    addDurationEdit->setValue(180);
    selectedMP3FilePath.clear();
    if (selectedFileLabel) {
        selectedFileLabel->setText("Файл не выбран");
        selectedFileLabel->setStyleSheet("color: gray; margin: 5px;");
    }
}

QWidget* MainWindow::createMainCatalogScreen() {
    QWidget *catalogWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(catalogWidget);

    // Заголовок
    QLabel *titleLabel = new QLabel("Музыкальный каталог");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // Таблица треков
    trackTable = new QTableWidget;
    trackTable->setColumnCount(7);
    trackTable->setHorizontalHeaderLabels({"Название", "Исполнитель", "Альбом", "Год", "Жанр", "Длительность", "Действия"});
    trackTable->horizontalHeader()->setStretchLastSection(false);
    trackTable->setColumnWidth(6, 120); // Ширина столбца с кнопками
    trackTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    trackTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    trackTable->setSelectionMode(QAbstractItemView::SingleSelection);
    trackTable->setSortingEnabled(true);
    trackTable->horizontalHeader()->setSortIndicatorShown(true);
    trackTable->horizontalHeader()->setSectionsClickable(true);
    trackTable->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);

    // Панель управления
    QHBoxLayout *controlLayout = new QHBoxLayout;
    QPushButton *addButton = new QPushButton("Добавить трек");
    QPushButton *yandexSearchButton = new QPushButton("Поиск в Яндекс Музыке");

    controlLayout->addWidget(addButton);
    controlLayout->addWidget(yandexSearchButton);
    controlLayout->addStretch();

    layout->addWidget(titleLabel);
    layout->addLayout(controlLayout);

    // Правая панель фильтров (вместо отдельной кнопки/экрана)
    QWidget *filtersPanel = new QWidget;
    filtersPanel->setMinimumWidth(260);
    QVBoxLayout *filtersLayout = new QVBoxLayout(filtersPanel);

    QLabel *filtersTitle = new QLabel("Фильтры");
    filtersTitle->setStyleSheet("font-size: 16px; font-weight: bold;");

    QFormLayout *filterForm = new QFormLayout;
    searchTitleEdit = new QLineEdit;
    searchArtistEdit = new QLineEdit;
    searchAlbumEdit = new QLineEdit;
    searchGenreEdit = new QLineEdit;
    searchMinYear = new QSpinBox;
    searchMinYear->setRange(1900, 2100);
    searchMinYear->setSpecialValueText("Любой");
    searchMaxYear = new QSpinBox;
    searchMaxYear->setRange(1900, 2100);
    searchMaxYear->setSpecialValueText("Любой");
    searchMinDuration = new QSpinBox;
    searchMinDuration->setRange(1, 3600);
    searchMinDuration->setSpecialValueText("Любая");
    searchMinDuration->setSuffix(" сек");
    searchMaxDuration = new QSpinBox;
    searchMaxDuration->setRange(1, 3600);
    searchMaxDuration->setSpecialValueText("Любая");
    searchMaxDuration->setSuffix(" сек");

    // Инициализация значений по умолчанию, чтобы поиск сразу работал корректно
    searchTitleEdit->clear();
    searchArtistEdit->clear();
    searchAlbumEdit->clear();
    searchGenreEdit->clear();
    searchMinYear->setValue(1900);
    searchMaxYear->setValue(2100);
    searchMinDuration->setValue(1);
    searchMaxDuration->setValue(3600);

    filterForm->addRow("Название:", searchTitleEdit);
    filterForm->addRow("Исполнитель:", searchArtistEdit);
    filterForm->addRow("Альбом:", searchAlbumEdit);
    filterForm->addRow("Жанр:", searchGenreEdit);
    filterForm->addRow("Год от:", searchMinYear);
    filterForm->addRow("Год до:", searchMaxYear);
    filterForm->addRow("Длительность от:", searchMinDuration);
    filterForm->addRow("Длительность до:", searchMaxDuration);

    QHBoxLayout *filterButtons = new QHBoxLayout;
    QPushButton *applyFiltersBtn = new QPushButton("Поиск");
    QPushButton *resetFiltersBtn = new QPushButton("Сброс");
    filterButtons->addWidget(applyFiltersBtn);
    filterButtons->addWidget(resetFiltersBtn);
    filterButtons->addStretch();

    filtersLayout->addWidget(filtersTitle);
    filtersLayout->addLayout(filterForm);
    filtersLayout->addLayout(filterButtons);
    filtersLayout->addStretch();

    // Область содержимого: таблица + панель фильтров
    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(trackTable, /*stretch*/ 1);
    contentLayout->addWidget(filtersPanel);
    layout->addLayout(contentLayout);

    // Подключение сигналов
    connect(addButton, &QPushButton::clicked, this, &MainWindow::showAddTrack);
    connect(yandexSearchButton, &QPushButton::clicked, this, &MainWindow::searchYandexMusic);
    connect(applyFiltersBtn, &QPushButton::clicked, this, &MainWindow::searchTracks);
    connect(resetFiltersBtn, &QPushButton::clicked, this, [this]() {
        resetSearch();
        updateTrackTable();
    });
    connect(trackTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::openTrackFile);
    // Сортировка по клику по заголовку работает автоматически при setSortingEnabled(true)

    return catalogWidget;
}

QWidget* MainWindow::createAddTrackScreen() {
    QWidget *addWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(addWidget);

    QLabel *titleLabel = new QLabel("Добавление нового трека");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin: 10px;");

    // Кнопка выбора файла
    QPushButton *selectFileButton = new QPushButton("Выбрать MP3 файл");
    selectFileButton->setStyleSheet("QPushButton { padding: 10px; font-size: 14px; }");
    connect(selectFileButton, &QPushButton::clicked, this, &MainWindow::onMP3FileSelected);

    // Метка для отображения выбранного файла
    selectedFileLabel = new QLabel("Файл не выбран");
    selectedFileLabel->setStyleSheet("color: gray; margin: 5px;");

    QFormLayout *formLayout = new QFormLayout;

    addTitleEdit = new QLineEdit;
    addTitleEdit->setReadOnly(false);
    addArtistEdit = new QLineEdit;
    addArtistEdit->setReadOnly(false);
    addAlbumEdit = new QLineEdit;
    addYearEdit = new QSpinBox;
    addYearEdit->setRange(1900, 2100);
    addYearEdit->setValue(2024);
    addGenreEdit = new QComboBox;
    populateGenreComboBox(addGenreEdit);
    addDurationEdit = new QSpinBox;
    addDurationEdit->setRange(1, 3600);
    addDurationEdit->setSuffix(" сек");

    formLayout->addRow("Название *:", addTitleEdit);
    formLayout->addRow("Исполнитель *:", addArtistEdit);
    formLayout->addRow("Альбом:", addAlbumEdit);
    formLayout->addRow("Год:", addYearEdit);
    formLayout->addRow("Жанр:", addGenreEdit);
    formLayout->addRow("Длительность:", addDurationEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *saveButton = new QPushButton("Сохранить");
    QPushButton *cancelButton = new QPushButton("Назад");

    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    layout->addWidget(titleLabel);
    layout->addWidget(selectFileButton);
    layout->addWidget(selectedFileLabel);
    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    connect(saveButton, &QPushButton::clicked, this, &MainWindow::addNewTrack);
    connect(cancelButton, &QPushButton::clicked, this, &MainWindow::showMainCatalog);

    return addWidget;
}

QWidget* MainWindow::createEditTrackScreen() {
    QWidget *editWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(editWidget);

    QLabel *titleLabel = new QLabel("Редактирование трека");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin: 10px;");

    QFormLayout *formLayout = new QFormLayout;

    editTitleEdit = new QLineEdit;
    editArtistEdit = new QLineEdit;
    editAlbumEdit = new QLineEdit;
    editYearEdit = new QSpinBox;
    editYearEdit->setRange(1900, 2100);
    editGenreEdit = new QComboBox;
    populateGenreComboBox(editGenreEdit);
    editDurationEdit = new QSpinBox;
    editDurationEdit->setRange(1, 3600);
    editDurationEdit->setSuffix(" сек");

    formLayout->addRow("Название *:", editTitleEdit);
    formLayout->addRow("Исполнитель *:", editArtistEdit);
    formLayout->addRow("Альбом:", editAlbumEdit);
    formLayout->addRow("Год:", editYearEdit);
    formLayout->addRow("Жанр:", editGenreEdit);
    formLayout->addRow("Длительность:", editDurationEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *saveButton = new QPushButton("Сохранить изменения");
    QPushButton *cancelButton = new QPushButton("Отмена");

    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    layout->addWidget(titleLabel);
    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    connect(saveButton, &QPushButton::clicked, [this]() {
        if (editTitleEdit->text().isEmpty() || editArtistEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Заполните обязательные поля (Название и Исполнитель)");
            return;
        }

        // Создаем обновленный трек
        Track updatedTrack(currentTrackId,
                           editTitleEdit->text(),
                           editArtistEdit->text(),
                           editAlbumEdit->text(),
                           editYearEdit->value(),
                           editGenreEdit->currentText(),
                           editDurationEdit->value());

        // Обновляем в каталоге
        if (catalog.updateTrack(currentTrackId, updatedTrack)) {
            QMessageBox::information(this, "Успех", "Трек успешно обновлен!");
            autoSaveCatalog();
            showMainCatalog();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось обновить трек");
        }
    });

    connect(cancelButton, &QPushButton::clicked, this, &MainWindow::showMainCatalog);

    return editWidget;
}

void MainWindow::searchYandexMusic()
{
    // Создаем диалог для поиска
    if (!yandexSearchDialog) {
        yandexSearchDialog = new QDialog(this);
        yandexSearchDialog->setWindowTitle("Поиск в Яндекс Музыке");
        yandexSearchDialog->setMinimumSize(600, 500);
        
        QVBoxLayout *dialogLayout = new QVBoxLayout(yandexSearchDialog);
        
        // Поле поиска
        QHBoxLayout *searchLayout = new QHBoxLayout;
        yandexSearchEdit = new QLineEdit;
        yandexSearchEdit->setPlaceholderText("Введите название трека или исполнителя...");
        QPushButton *searchBtn = new QPushButton("Поиск");
        searchLayout->addWidget(yandexSearchEdit);
        searchLayout->addWidget(searchBtn);
        
        // Список результатов
        yandexResultsList = new QListWidget;
        yandexResultsList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        
        // Кнопки
        QHBoxLayout *buttonLayout = new QHBoxLayout;
        QPushButton *importBtn = new QPushButton("Импортировать выбранные");
        QPushButton *closeBtn = new QPushButton("Закрыть");
        buttonLayout->addWidget(importBtn);
        buttonLayout->addStretch();
        buttonLayout->addWidget(closeBtn);
        
        dialogLayout->addLayout(searchLayout);
        dialogLayout->addWidget(yandexResultsList);
        dialogLayout->addLayout(buttonLayout);
        
        connect(searchBtn, &QPushButton::clicked, [this]() {
            QString query = yandexSearchEdit->text().trimmed();
            if (!query.isEmpty()) {
                yandexResultsList->clear();
                yandexResultsList->addItem("Поиск...");
                yandexIntegrator->searchAndImportTracks(query);
            }
        });
        
        connect(importBtn, &QPushButton::clicked, this, &MainWindow::importSelectedYandexTracks);
        connect(closeBtn, &QPushButton::clicked, yandexSearchDialog, &QDialog::accept);
        
        // Поиск по Enter
        connect(yandexSearchEdit, &QLineEdit::returnPressed, searchBtn, &QPushButton::click);
    }
    
    yandexSearchEdit->clear();
    yandexResultsList->clear();
    currentYandexTracks.clear();
    yandexSearchDialog->exec();
}

void MainWindow::onYandexTracksFound(const QList<YandexTrack>& tracks)
{
    currentYandexTracks = tracks;
    yandexResultsList->clear();
    
    if (tracks.isEmpty()) {
        yandexResultsList->addItem("Треки не найдены");
        return;
    }
    
    for (const YandexTrack& track : tracks) {
        QString itemText = QString("%1 - %2")
                          .arg(track.artist)
                          .arg(track.title);
        
        if (!track.album.isEmpty()) {
            itemText += QString(" [%1]").arg(track.album);
        }
        
        if (track.year > 0) {
            itemText += QString(" (%1)").arg(track.year);
        }
        
        if (track.duration > 0) {
            int minutes = track.duration / 60;
            int seconds = track.duration % 60;
            itemText += QString(" - %1:%2")
                        .arg(minutes, 2, 10, QChar('0'))
                        .arg(seconds, 2, 10, QChar('0'));
        }
        
        yandexResultsList->addItem(itemText);
    }
}

void MainWindow::onYandexTrackImported(const QString& trackTitle)
{
    // Обновляем таблицу после импорта
    updateTrackTable();
}

void MainWindow::onYandexError(const QString& errorMessage)
{
    QMessageBox::warning(this, "Ошибка Яндекс Музыки", errorMessage);
    if (yandexResultsList) {
        yandexResultsList->clear();
        yandexResultsList->addItem("Ошибка: " + errorMessage);
    }
}

void MainWindow::importSelectedYandexTracks()
{
    QList<QListWidgetItem*> selectedItems = yandexResultsList->selectedItems();
    
    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, "Информация", "Выберите треки для импорта");
        return;
    }
    
    int importedCount = 0;
    
    for (QListWidgetItem* item : selectedItems) {
        int index = yandexResultsList->row(item);
        if (index >= 0 && index < currentYandexTracks.size()) {
            yandexIntegrator->importTrack(currentYandexTracks[index]);
            importedCount++;
        }
    }
    
    if (importedCount > 0) {
        QMessageBox::information(this, "Успех", 
                                 QString("Импортировано треков: %1").arg(importedCount));
        updateTrackTable();
        // Удаляем импортированные элементы из списка
        for (QListWidgetItem* item : selectedItems) {
            delete item;
        }
    }
}
