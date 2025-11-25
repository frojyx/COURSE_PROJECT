// mainwindow.cpp
#include "mainwindow.h"
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
    : QMainWindow(parent)
{
    setWindowTitle("Музыкальный каталог");
    setMinimumSize(1000, 700);

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    // Инициализируем интегратор Яндекс Музыки
    yandexUI.integrator = new YandexMusicIntegrator(&catalog, this);
    connect(yandexUI.integrator, &YandexMusicIntegrator::tracksFound,
            this, &MainWindow::onYandexTracksFound);
    connect(yandexUI.integrator, &YandexMusicIntegrator::trackImported,
            this, &MainWindow::onYandexTrackImported);
    connect(yandexUI.integrator, &YandexMusicIntegrator::errorOccurred,
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
    if (addTrackUI.selectedMP3FilePath.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите MP3 файл");
        return;
    }

    if (addTrackUI.addTitleEdit->text().isEmpty() || addTrackUI.addArtistEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните обязательные поля (Название и Исполнитель)");
        return;
    }

    // Добавляем трек в каталог
    int newId = catalog.getNextId();

    // Переименовываем файл
    QString newFileName = MP3FileManager::createNewFileName(newId,
                                                            addTrackUI.addTitleEdit->text(),
                                                            addTrackUI.addArtistEdit->text(),
                                                            addTrackUI.addAlbumEdit->text(),
                                                            addTrackUI.addYearEdit->value(),
                                                            addTrackUI.addGenreEdit->currentText(),
                                                            addTrackUI.addDurationEdit->value());

    QString newFilePath = QFileInfo(addTrackUI.selectedMP3FilePath).absolutePath() + "/" + newFileName;

    if (!MP3FileManager::renameFile(addTrackUI.selectedMP3FilePath, newFileName)) {
        QMessageBox::warning(this, "Предупреждение", "Не удалось переименовать файл");
        // Используем старый путь
        newFilePath = addTrackUI.selectedMP3FilePath;
    }

    catalog.addTrack(addTrackUI.addTitleEdit->text(),
                     addTrackUI.addArtistEdit->text(),
                     addTrackUI.addAlbumEdit->text(),
                     addTrackUI.addYearEdit->value(),
                     addTrackUI.addGenreEdit->currentText(),
                     addTrackUI.addDurationEdit->value(),
                     newFilePath);

    // Автосохранение
    autoSaveCatalog();

    QMessageBox::information(this, "Успех", "Трек успешно добавлен!");

    showMainCatalog();
}

void MainWindow::searchTracks() {
    QString title = searchUI.searchTitleEdit->text();
    QString artist = searchUI.searchArtistEdit->text();
    QString album = searchUI.searchAlbumEdit->text();
    QString genre = searchUI.searchGenreEdit->text();

    int minYear = searchUI.searchMinYear->value();
    int maxYear = searchUI.searchMaxYear->value();
    int minDuration = searchUI.searchMinDuration->value();
    int maxDuration = searchUI.searchMaxDuration->value();

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

    for (int row = 0; row < searchUI.trackTable->rowCount(); ++row) {
        const QTableWidgetItem *titleItem = searchUI.trackTable->item(row, 0);
        if (!titleItem) continue;
        int id = titleItem->data(Qt::UserRole).toInt();
        bool isMatch = resultIds.contains(id);

        // Применяем фон ко всем ячейкам строки, кроме виджета действий
        for (int col = 0; col < 6; ++col) {
            QTableWidgetItem *cell = searchUI.trackTable->item(row, col);
            if (!cell) continue;
            cell->setBackground(isMatch ? QBrush(highlightColor) : QBrush());
        }
    }

    if (results.isEmpty()) {
        QMessageBox::information(this, "Поиск", "Треки не найдены");
    }
}


void MainWindow::autoSaveCatalog() const {
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
        addTrackUI.selectedMP3FilePath.clear();
        if (addTrackUI.selectedFileLabel) {
            addTrackUI.selectedFileLabel->setText("Файл не выбран");
            addTrackUI.selectedFileLabel->setStyleSheet("color: gray; margin: 5px;");
        }
        return;
    }

    addTrackUI.selectedMP3FilePath = fileName;
    QString fileBaseName = QFileInfo(fileName).fileName();

    // Обновляем метку выбранного файла
    if (addTrackUI.selectedFileLabel) {
        addTrackUI.selectedFileLabel->setText("Выбран: " + fileBaseName);
        addTrackUI.selectedFileLabel->setStyleSheet("color: green; margin: 5px;");
    }

    // Переменные для всех полей
    QString title;
    QString artist;

    // Сначала пытаемся парсить имя файла (полный формат: ID.название.исполнитель.альбом.год.жанр.длительность.mp3)
    QString parsedAlbum;
    QString parsedGenre;
    int parsedYear = 2024;
    int parsedDuration = 180;

    if (MP3FileManager::parseFileName(fileBaseName, title, artist, &parsedAlbum, &parsedYear, &parsedGenre, &parsedDuration)) {
        addTrackUI.addTitleEdit->setText(title);
        addTrackUI.addArtistEdit->setText(artist);
        fillFormFromParsedFileName(fileBaseName, title, artist, parsedAlbum, parsedYear, parsedGenre, parsedDuration);
    } else {
        fillFormFromMP3Metadata(fileName);
    }
}

void MainWindow::fillFormFromParsedFileName(const QString& fileBaseName, [[maybe_unused]] const QString& title, 
                                            [[maybe_unused]] const QString& artist, const QString& parsedAlbum,
                                            int parsedYear, const QString& parsedGenre, int parsedDuration) {
    QString baseName = QFileInfo(fileBaseName).baseName();
    if (baseName.isEmpty() || !baseName[0].isDigit()) {
        return;
    }
    
    if (QStringList parts = baseName.split('.'); parts.size() < 7) {
        return;
    }
    
    // Полный формат найден - заполняем все поля
                if (!parsedAlbum.isEmpty()) {
                    addTrackUI.addAlbumEdit->setText(parsedAlbum);
                }
                if (parsedYear > 1900 && parsedYear < 2100) {
                    addTrackUI.addYearEdit->setValue(parsedYear);
                }
                if (!parsedGenre.isEmpty()) {
                    if (int index = addTrackUI.addGenreEdit->findText(parsedGenre, Qt::MatchExactly); index >= 0) {
                        addTrackUI.addGenreEdit->setCurrentIndex(index);
                    } else {
                        addTrackUI.addGenreEdit->setCurrentIndex(0);
                    }
                }
                if (parsedDuration > 0) {
                    addTrackUI.addDurationEdit->setValue(parsedDuration);
                }
}

void MainWindow::fillFormFromMP3Metadata(const QString& fileName) {
    QString titleFromMeta;
    QString artistFromMeta;
        if (MP3FileManager::readMP3Metadata(fileName, titleFromMeta, artistFromMeta)) {
            addTrackUI.addTitleEdit->setText(titleFromMeta);
            addTrackUI.addArtistEdit->setText(artistFromMeta);
    }
}

void MainWindow::openTrackFile(int row, int column) {
    Q_UNUSED(column)
    if (row < 0) return;

    const QTableWidgetItem *item = searchUI.trackTable->item(row, 0);
    if (!item) return;
    int trackId = item->data(Qt::UserRole).toInt();
    const Track* track = catalog.findTrackById(trackId);

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
    const Track* track = catalog.findTrackById(trackId);

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
    const Track* track = catalog.findTrackById(trackId);

    if (!track) {
        QMessageBox::warning(this, "Ошибка", "Трек не найден");
        return;
    }

    // Заполняем форму редактирования данными трека
    editTrackUI.editTitleEdit->setText(track->getTitle());
    editTrackUI.editArtistEdit->setText(track->getArtist());
    editTrackUI.editAlbumEdit->setText(track->getAlbum());
    editTrackUI.editYearEdit->setValue(track->getYear());
    // Пытаемся найти жанр в списке
    if (int index = editTrackUI.editGenreEdit->findText(track->getGenre(), Qt::MatchExactly); index >= 0) {
        editTrackUI.editGenreEdit->setCurrentIndex(index);
    } else {
        // Если жанр не найден в списке, устанавливаем текст (для редактируемого комбобокса)
        editTrackUI.editGenreEdit->setCurrentText(track->getGenre());
    }
    editTrackUI.editDurationEdit->setValue(track->getDuration());

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
    searchUI.searchTitleEdit->clear();
    searchUI.searchArtistEdit->clear();
    searchUI.searchAlbumEdit->clear();
    searchUI.searchGenreEdit->clear();
    searchUI.searchMinYear->setValue(1900);
    searchUI.searchMaxYear->setValue(2100);
    searchUI.searchMinDuration->setValue(1);
    searchUI.searchMaxDuration->setValue(3600);
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
    bool sortingWasEnabled = searchUI.trackTable->isSortingEnabled();
    searchUI.trackTable->setSortingEnabled(false);

    searchUI.trackTable->setRowCount(tracks.size());

    for (int i = 0; i < tracks.size(); ++i) {
        const Track& track = tracks[i];

        // Сохраняем trackId в данных первого столбца (название)
        auto *titleItem = new QTableWidgetItem(track.getTitle());
        titleItem->setData(Qt::UserRole, track.getId());
        searchUI.trackTable->setItem(i, 0, titleItem);

        searchUI.trackTable->setItem(i, 1, new QTableWidgetItem(track.getArtist()));
        searchUI.trackTable->setItem(i, 2, new QTableWidgetItem(track.getAlbum()));
        searchUI.trackTable->setItem(i, 3, new QTableWidgetItem(QString::number(track.getYear())));
        searchUI.trackTable->setItem(i, 4, new QTableWidgetItem(track.getGenre()));
        searchUI.trackTable->setItem(i, 5, new QTableWidgetItem(track.getFormattedDuration()));

        // Создаем виджет с кнопками для столбца действий
        auto *actionWidget = new QWidget();
        auto *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(3, 2, 3, 2);
        actionLayout->setSpacing(5);

        auto *playButton = new QPushButton("▶");
        playButton->setToolTip("Проиграть");
        playButton->setFixedSize(28, 24);
        playButton->setStyleSheet("QPushButton { font-size: 12px; }");

        auto *editButton = new QPushButton("✎");
        editButton->setToolTip("Редактировать");
        editButton->setFixedSize(28, 24);
        editButton->setStyleSheet("QPushButton { font-size: 12px; }");

        auto *deleteButton = new QPushButton("✗");
        deleteButton->setToolTip("Удалить");
        deleteButton->setFixedSize(28, 24);
        deleteButton->setStyleSheet("QPushButton { font-size: 12px; color: red; }");

        actionLayout->addWidget(playButton);
        actionLayout->addWidget(editButton);
        actionLayout->addWidget(deleteButton);
        actionLayout->addStretch();
        actionLayout->setAlignment(Qt::AlignCenter);

        actionWidget->setLayout(actionLayout);
        searchUI.trackTable->setCellWidget(i, 6, actionWidget);

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
    searchUI.trackTable->setSortingEnabled(sortingWasEnabled);
}

QStringList MainWindow::getGenreList() const {
    return GenreManager::getGenreList();
}

void MainWindow::populateGenreComboBox(QComboBox *comboBox) const {
    if (!comboBox) return;
    comboBox->clear();
    comboBox->addItem(""); // Пустой элемент для сброса
    comboBox->addItems(getGenreList());
    comboBox->setEditable(false); // Только выбор из списка, без возможности ввода вручную
}

void MainWindow::clearAddTrackForm() {
    addTrackUI.addTitleEdit->clear();
    addTrackUI.addArtistEdit->clear();
    addTrackUI.addAlbumEdit->clear();
    addTrackUI.addYearEdit->setValue(2024);
    addTrackUI.addGenreEdit->setCurrentIndex(0); // Сбрасываем на первый элемент
    addTrackUI.addDurationEdit->setValue(180);
    addTrackUI.selectedMP3FilePath.clear();
    if (addTrackUI.selectedFileLabel) {
        addTrackUI.selectedFileLabel->setText("Файл не выбран");
        addTrackUI.selectedFileLabel->setStyleSheet("color: gray; margin: 5px;");
    }
}

QWidget* MainWindow::createMainCatalogScreen() {
    auto *catalogWidget = new QWidget;
    auto *layout = new QVBoxLayout(catalogWidget);

    // Заголовок
    auto *titleLabel = new QLabel("Музыкальный каталог");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // Таблица треков
    searchUI.trackTable = new QTableWidget;
    searchUI.trackTable->setColumnCount(7);
    searchUI.trackTable->setHorizontalHeaderLabels({"Название", "Исполнитель", "Альбом", "Год", "Жанр", "Длительность", "Действия"});
    searchUI.trackTable->horizontalHeader()->setStretchLastSection(false);
    searchUI.trackTable->setColumnWidth(6, 120); // Ширина столбца с кнопками
    searchUI.trackTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    searchUI.trackTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    searchUI.trackTable->setSelectionMode(QAbstractItemView::SingleSelection);
    searchUI.trackTable->setSortingEnabled(true);
    searchUI.trackTable->horizontalHeader()->setSortIndicatorShown(true);
    searchUI.trackTable->horizontalHeader()->setSectionsClickable(true);
    searchUI.trackTable->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);

    // Панель управления
    auto *controlLayout = new QHBoxLayout;
    auto *addButton = new QPushButton("Добавить трек");
    auto *yandexSearchButton = new QPushButton("Поиск в Яндекс Музыке");

    controlLayout->addWidget(addButton);
    controlLayout->addWidget(yandexSearchButton);
    controlLayout->addStretch();

    layout->addWidget(titleLabel);
    layout->addLayout(controlLayout);

    // Правая панель фильтров (вместо отдельной кнопки/экрана)
    auto *filtersPanel = new QWidget;
    filtersPanel->setMinimumWidth(260);
    auto *filtersLayout = new QVBoxLayout(filtersPanel);

    auto *filtersTitle = new QLabel("Фильтры");
    filtersTitle->setStyleSheet("font-size: 16px; font-weight: bold;");

    auto *filterForm = new QFormLayout;
    searchUI.searchTitleEdit = new QLineEdit;
    searchUI.searchArtistEdit = new QLineEdit;
    searchUI.searchAlbumEdit = new QLineEdit;
    searchUI.searchGenreEdit = new QLineEdit;
    searchUI.searchMinYear = new QSpinBox;
    searchUI.searchMinYear->setRange(1900, 2100);
    searchUI.searchMinYear->setSpecialValueText("Любой");
    searchUI.searchMaxYear = new QSpinBox;
    searchUI.searchMaxYear->setRange(1900, 2100);
    searchUI.searchMaxYear->setSpecialValueText("Любой");
    searchUI.searchMinDuration = new QSpinBox;
    searchUI.searchMinDuration->setRange(1, 3600);
    searchUI.searchMinDuration->setSpecialValueText("Любая");
    searchUI.searchMinDuration->setSuffix(" сек");
    searchUI.searchMaxDuration = new QSpinBox;
    searchUI.searchMaxDuration->setRange(1, 3600);
    searchUI.searchMaxDuration->setSpecialValueText("Любая");
    searchUI.searchMaxDuration->setSuffix(" сек");

    // Инициализация значений по умолчанию, чтобы поиск сразу работал корректно
    searchUI.searchTitleEdit->clear();
    searchUI.searchArtistEdit->clear();
    searchUI.searchAlbumEdit->clear();
    searchUI.searchGenreEdit->clear();
    searchUI.searchMinYear->setValue(1900);
    searchUI.searchMaxYear->setValue(2100);
    searchUI.searchMinDuration->setValue(1);
    searchUI.searchMaxDuration->setValue(3600);

    filterForm->addRow("Название:", searchUI.searchTitleEdit);
    filterForm->addRow("Исполнитель:", searchUI.searchArtistEdit);
    filterForm->addRow("Альбом:", searchUI.searchAlbumEdit);
    filterForm->addRow("Жанр:", searchUI.searchGenreEdit);
    filterForm->addRow("Год от:", searchUI.searchMinYear);
    filterForm->addRow("Год до:", searchUI.searchMaxYear);
    filterForm->addRow("Длительность от:", searchUI.searchMinDuration);
    filterForm->addRow("Длительность до:", searchUI.searchMaxDuration);

    auto *filterButtons = new QHBoxLayout;
    auto *applyFiltersBtn = new QPushButton("Поиск");
    auto *resetFiltersBtn = new QPushButton("Сброс");
    filterButtons->addWidget(applyFiltersBtn);
    filterButtons->addWidget(resetFiltersBtn);
    filterButtons->addStretch();

    filtersLayout->addWidget(filtersTitle);
    filtersLayout->addLayout(filterForm);
    filtersLayout->addLayout(filterButtons);
    filtersLayout->addStretch();

    // Область содержимого: таблица + панель фильтров
    auto *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(searchUI.trackTable, /*stretch*/ 1);
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
    connect(searchUI.trackTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::openTrackFile);
    // Сортировка по клику по заголовку работает автоматически при setSortingEnabled(true)

    return catalogWidget;
}

QWidget* MainWindow::createAddTrackScreen() {
    auto *addWidget = new QWidget;
    auto *layout = new QVBoxLayout(addWidget);

    auto *titleLabel = new QLabel("Добавление нового трека");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin: 10px;");

    // Кнопка выбора файла
    auto *selectFileButton = new QPushButton("Выбрать MP3 файл");
    selectFileButton->setStyleSheet("QPushButton { padding: 10px; font-size: 14px; }");
    connect(selectFileButton, &QPushButton::clicked, this, &MainWindow::onMP3FileSelected);

    // Метка для отображения выбранного файла
    addTrackUI.selectedFileLabel = new QLabel("Файл не выбран");
    addTrackUI.selectedFileLabel->setStyleSheet("color: gray; margin: 5px;");

    auto *formLayout = new QFormLayout;

    addTrackUI.addTitleEdit = new QLineEdit;
    addTrackUI.addTitleEdit->setReadOnly(false);
    addTrackUI.addArtistEdit = new QLineEdit;
    addTrackUI.addArtistEdit->setReadOnly(false);
    addTrackUI.addAlbumEdit = new QLineEdit;
    addTrackUI.addYearEdit = new QSpinBox;
    addTrackUI.addYearEdit->setRange(1900, 2100);
    addTrackUI.addYearEdit->setValue(2024);
    addTrackUI.addGenreEdit = new QComboBox;
    populateGenreComboBox(addTrackUI.addGenreEdit);
    addTrackUI.addDurationEdit = new QSpinBox;
    addTrackUI.addDurationEdit->setRange(1, 3600);
    addTrackUI.addDurationEdit->setSuffix(" сек");

    formLayout->addRow("Название *:", addTrackUI.addTitleEdit);
    formLayout->addRow("Исполнитель *:", addTrackUI.addArtistEdit);
    formLayout->addRow("Альбом:", addTrackUI.addAlbumEdit);
    formLayout->addRow("Год:", addTrackUI.addYearEdit);
    formLayout->addRow("Жанр:", addTrackUI.addGenreEdit);
    formLayout->addRow("Длительность:", addTrackUI.addDurationEdit);

    auto *buttonLayout = new QHBoxLayout;
    auto *saveButton = new QPushButton("Сохранить");
    auto *cancelButton = new QPushButton("Назад");

    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    layout->addWidget(titleLabel);
    layout->addWidget(selectFileButton);
    layout->addWidget(addTrackUI.selectedFileLabel);
    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    connect(saveButton, &QPushButton::clicked, this, &MainWindow::addNewTrack);
    connect(cancelButton, &QPushButton::clicked, this, &MainWindow::showMainCatalog);

    return addWidget;
}

QWidget* MainWindow::createEditTrackScreen() {
    auto *editWidget = new QWidget;
    auto *layout = new QVBoxLayout(editWidget);

    auto *titleLabel = new QLabel("Редактирование трека");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin: 10px;");

    auto *formLayout = new QFormLayout;

    editTrackUI.editTitleEdit = new QLineEdit;
    editTrackUI.editArtistEdit = new QLineEdit;
    editTrackUI.editAlbumEdit = new QLineEdit;
    editTrackUI.editYearEdit = new QSpinBox;
    editTrackUI.editYearEdit->setRange(1900, 2100);
    editTrackUI.editGenreEdit = new QComboBox;
    populateGenreComboBox(editTrackUI.editGenreEdit);
    editTrackUI.editDurationEdit = new QSpinBox;
    editTrackUI.editDurationEdit->setRange(1, 3600);
    editTrackUI.editDurationEdit->setSuffix(" сек");

    formLayout->addRow("Название *:", editTrackUI.editTitleEdit);
    formLayout->addRow("Исполнитель *:", editTrackUI.editArtistEdit);
    formLayout->addRow("Альбом:", editTrackUI.editAlbumEdit);
    formLayout->addRow("Год:", editTrackUI.editYearEdit);
    formLayout->addRow("Жанр:", editTrackUI.editGenreEdit);
    formLayout->addRow("Длительность:", editTrackUI.editDurationEdit);

    auto *buttonLayout = new QHBoxLayout;
    auto *saveButton = new QPushButton("Сохранить изменения");
    auto *cancelButton = new QPushButton("Отмена");

    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    layout->addWidget(titleLabel);
    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    connect(saveButton, &QPushButton::clicked, [this]() {
        if (editTrackUI.editTitleEdit->text().isEmpty() || editTrackUI.editArtistEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Заполните обязательные поля (Название и Исполнитель)");
            return;
        }

        // Создаем обновленный трек
        Track updatedTrack(currentTrackId,
                           editTrackUI.editTitleEdit->text(),
                           editTrackUI.editArtistEdit->text(),
                           editTrackUI.editAlbumEdit->text(),
                           editTrackUI.editYearEdit->value(),
                           editTrackUI.editGenreEdit->currentText(),
                           editTrackUI.editDurationEdit->value());

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
    if (!yandexUI.searchDialog) {
        yandexUI.searchDialog = new QDialog(this);
        yandexUI.searchDialog->setWindowTitle("Поиск в Яндекс Музыке");
        yandexUI.searchDialog->setMinimumSize(600, 500);
        
        auto *dialogLayout = new QVBoxLayout(yandexUI.searchDialog);
        
        // Поле поиска
        auto *searchLayout = new QHBoxLayout;
        yandexUI.searchEdit = new QLineEdit;
        yandexUI.searchEdit->setPlaceholderText("Введите название трека или исполнителя...");
        auto *searchBtn = new QPushButton("Поиск");
        searchLayout->addWidget(yandexUI.searchEdit);
        searchLayout->addWidget(searchBtn);
        
        // Список результатов
        yandexUI.resultsList = new QListWidget;
        yandexUI.resultsList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        
        // Кнопки
        auto *buttonLayout = new QHBoxLayout;
        auto *importBtn = new QPushButton("Импортировать выбранные");
        auto *closeBtn = new QPushButton("Закрыть");
        buttonLayout->addWidget(importBtn);
        buttonLayout->addStretch();
        buttonLayout->addWidget(closeBtn);
        
        dialogLayout->addLayout(searchLayout);
        dialogLayout->addWidget(yandexUI.resultsList);
        dialogLayout->addLayout(buttonLayout);
        
        connect(searchBtn, &QPushButton::clicked, [this]() {
            QString query = yandexUI.searchEdit->text().trimmed();
            if (!query.isEmpty()) {
                yandexUI.resultsList->clear();
                yandexUI.resultsList->addItem("Поиск...");
                yandexUI.integrator->searchAndImportTracks(query);
            }
        });
        
        connect(importBtn, &QPushButton::clicked, this, &MainWindow::importSelectedYandexTracks);
        connect(closeBtn, &QPushButton::clicked, yandexUI.searchDialog, &QDialog::accept);
        
        // Поиск по Enter
        connect(yandexUI.searchEdit, &QLineEdit::returnPressed, searchBtn, &QPushButton::click);
    }
    
    yandexUI.searchEdit->clear();
    yandexUI.resultsList->clear();
    yandexUI.currentTracks.clear();
    yandexUI.searchDialog->exec();
}

void MainWindow::onYandexTracksFound(const QList<YandexTrack>& tracks)
{
    yandexUI.currentTracks = tracks;
    yandexUI.resultsList->clear();
    
    if (tracks.isEmpty()) {
        yandexUI.resultsList->addItem("Треки не найдены");
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
        
        yandexUI.resultsList->addItem(itemText);
    }
}

void MainWindow::onYandexTrackImported(const QString& [[maybe_unused]] trackTitle)
{
    // Обновляем таблицу после импорта
    updateTrackTable();
}

void MainWindow::onYandexError(const QString& errorMessage)
{
    QMessageBox::warning(this, "Ошибка Яндекс Музыки", errorMessage);
    if (yandexUI.resultsList) {
        yandexUI.resultsList->clear();
        yandexUI.resultsList->addItem("Ошибка: " + errorMessage);
    }
}

void MainWindow::importSelectedYandexTracks()
{
    QList<QListWidgetItem*> selectedItems = yandexUI.resultsList->selectedItems();
    
    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, "Информация", "Выберите треки для импорта");
        return;
    }
    
    int importedCount = 0;
    
    for (const QListWidgetItem* item : selectedItems) {
        int index = yandexUI.resultsList->row(item);
        if (index >= 0 && index < yandexUI.currentTracks.size()) {
            yandexUI.integrator->importTrack(yandexUI.currentTracks[index]);
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
