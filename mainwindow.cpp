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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentTrackId(-1), searchResultsWindow(nullptr)
{
    setWindowTitle("Музыкальный каталог");
    setMinimumSize(1000, 700);

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    // Создаем экраны
    stackedWidget->addWidget(createLoginScreen());
    stackedWidget->addWidget(createMainCatalogScreen());
    stackedWidget->addWidget(createAddTrackScreen());
    stackedWidget->addWidget(createSearchScreen());
    stackedWidget->addWidget(createTrackDetailsScreen());
    stackedWidget->addWidget(createEditTrackScreen());

    // Автозагрузка каталога при запуске
    autoLoadCatalog();

    // Показываем экран входа
    showLoginScreen();
}

void MainWindow::showLoginScreen() {
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::showMainCatalog() {
    updateTrackTable();
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::showAddTrack() {
    clearAddTrackForm();
    stackedWidget->setCurrentIndex(2);
}

void MainWindow::showSearchFilters() {
    stackedWidget->setCurrentIndex(3);
}

void MainWindow::showTrackDetails(int row) {
    stackedWidget->setCurrentIndex(4);
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
    QList<Track> results = catalog.searchTracksWithFilters(
        title, artist, album, genre,
        minYear, maxYear, minDuration, maxDuration
        );

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

void MainWindow::generateReport() {
    QString report = ReportGenerator::generateReport(catalog);

    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить отчет", "", "Text Files (*.txt)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << report;
            file.close();
            QMessageBox::information(this, "Успех", "Отчет сохранен");
        }
    }
}

void MainWindow::deleteSelectedTrack() {
    int currentRow = trackTable->currentRow();
    if (currentRow == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите трек для удаления");
        return;
    }

    QTableWidgetItem *item = trackTable->item(currentRow, 0);
    if (!item) {
        QMessageBox::warning(this, "Ошибка", "Трек не найден");
        return;
    }
    int trackId = item->data(Qt::UserRole).toInt();

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

void MainWindow::updateSelectedTrack() {
    int currentRow = trackTable->currentRow();
    if (currentRow == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите трек для редактирования");
        return;
    }

    QTableWidgetItem *item = trackTable->item(currentRow, 0);
    if (!item) {
        QMessageBox::warning(this, "Ошибка", "Трек не найден");
        return;
    }
    int trackId = item->data(Qt::UserRole).toInt();
    Track* track = catalog.findTrackById(trackId);

    if (track) {
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
            // Если жанр не найден в списке, устанавливаем пустой элемент
            editGenreEdit->setCurrentIndex(0);
        }
        editDurationEdit->setValue(track->getDuration());

        currentTrackId = trackId;
        stackedWidget->setCurrentIndex(5); // Экран редактирования
    } else {
        QMessageBox::warning(this, "Ошибка", "Трек не найден");
    }
}

void MainWindow::saveCatalog() {
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить каталог", "music_catalog.csv", "CSV Files (*.csv)");
    if (!fileName.isEmpty()) {
        if (FileManager::saveToCSV(catalog, fileName)) {
            QMessageBox::information(this, "Успех", "Каталог сохранен");
            autoSaveCatalog(); // Также сохраняем в автосохранение
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось сохранить каталог");
        }
    }
}

void MainWindow::loadCatalog() {
    QString fileName = QFileDialog::getOpenFileName(this, "Загрузить каталог", "", "CSV Files (*.csv)");
    if (!fileName.isEmpty()) {
        if (FileManager::loadFromCSV(catalog, fileName)) {
            QMessageBox::information(this, "Успех", "Каталог загружен");
            updateTrackTable();
            autoSaveCatalog(); // Сохраняем в автосохранение
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось загрузить каталог");
        }
    }
}

void MainWindow::autoSaveCatalog() {
    QString fileName = "catalog_autosave.csv";
    FileManager::saveToCSV(catalog, fileName);
}

void MainWindow::autoLoadCatalog() {
    QString fileName = "catalog_autosave.csv";
    if (QFile::exists(fileName)) {
        FileManager::loadFromCSV(catalog, fileName);
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

    if (track && !track->getFilePath().isEmpty()) {
        MP3FileManager::openMP3File(track->getFilePath());
    } else if (track) {
        // Если путь не сохранен, пытаемся найти файл
        QString filePath = mp3Manager.findFileByTrack(track->getId(),
                                                      track->getTitle(),
                                                      track->getArtist(),
                                                      track->getAlbum(),
                                                      track->getYear(),
                                                      track->getGenre(),
                                                      track->getDuration());
        if (!filePath.isEmpty()) {
            MP3FileManager::openMP3File(filePath);
        } else {
            QMessageBox::warning(this, "Ошибка", "Файл не найден");
        }
    }
}

void MainWindow::playTrack(int row) {
    if (row < 0 || row >= trackTable->rowCount()) return;

    QTableWidgetItem *item = trackTable->item(row, 0);
    if (!item) return;

    int trackId = item->data(Qt::UserRole).toInt();
    playTrackById(trackId);
}

void MainWindow::editTrackFromRow(int row) {
    if (row < 0 || row >= trackTable->rowCount()) {
        QMessageBox::warning(this, "Ошибка", "Неверный индекс строки");
        return;
    }

    QTableWidgetItem *item = trackTable->item(row, 0);
    if (!item) {
        QMessageBox::warning(this, "Ошибка", "Трек не найден");
        return;
    }

    int trackId = item->data(Qt::UserRole).toInt();
    editTrackById(trackId);
}

void MainWindow::deleteTrackFromRow(int row) {
    if (row < 0 || row >= trackTable->rowCount()) {
        QMessageBox::warning(this, "Ошибка", "Неверный индекс строки");
        return;
    }

    QTableWidgetItem *item = trackTable->item(row, 0);
    if (!item) {
        QMessageBox::warning(this, "Ошибка", "Трек не найден");
        return;
    }

    int trackId = item->data(Qt::UserRole).toInt();
    deleteTrackById(trackId);
}

void MainWindow::playTrackById(int trackId) {
    Track* track = catalog.findTrackById(trackId);

    if (!track) {
        QMessageBox::warning(this, "Ошибка", "Трек не найден");
        return;
    }

    if (!track->getFilePath().isEmpty()) {
        MP3FileManager::openMP3File(track->getFilePath());
    } else {
        // Если путь не сохранен, пытаемся найти файл
        QString filePath = mp3Manager.findFileByTrack(track->getId(),
                                                      track->getTitle(),
                                                      track->getArtist(),
                                                      track->getAlbum(),
                                                      track->getYear(),
                                                      track->getGenre(),
                                                      track->getDuration());
        if (!filePath.isEmpty()) {
            MP3FileManager::openMP3File(filePath);
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
    stackedWidget->setCurrentIndex(5); // Экран редактирования
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

void MainWindow::logout() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Выход",
                                                              "Вы уверены, что хотите выйти?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        showLoginScreen();
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

void MainWindow::sortTracks() {
    int sortIndex = sortComboBox->currentIndex();

    switch (sortIndex) {
    case 0: // Без сортировки
        // Не сортируем, просто обновляем таблицу
        break;
    case 1: // Название (А-Я)
        catalog.sortByTitle(true);
        break;
    case 2: // Название (Я-А)
        catalog.sortByTitle(false);
        break;
    case 3: // Исполнитель (А-Я)
        catalog.sortByArtist(true);
        break;
    case 4: // Исполнитель (Я-А)
        catalog.sortByArtist(false);
        break;
    case 5: // Год (по возрастанию)
        catalog.sortByYear(true);
        break;
    case 6: // Год (по убыванию)
        catalog.sortByYear(false);
        break;
    case 7: // Длительность (по возрастанию)
        catalog.sortByDuration(true);
        break;
    case 8: // Длительность (по убыванию)
        catalog.sortByDuration(false);
        break;
    }

    if (sortIndex != 0) {
        updateTrackTable();
    }
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

bool MainWindow::validateLogin(const QString &username, const QString &password) {
    return AuthenticationService::validateLogin(username, password);
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

Track MainWindow::getSelectedTrack() const {
    int currentRow = trackTable->currentRow();
    if (currentRow == -1) {
        return Track();
    }

    QTableWidgetItem *item = trackTable->item(currentRow, 0);
    if (!item) {
        return Track();
    }

    int trackId = item->data(Qt::UserRole).toInt();
    const Track* track = catalog.findTrackById(trackId);

    if (track) {
        return *track;
    }

    return Track();
}

QWidget* MainWindow::createLoginScreen() {
    QWidget *loginWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(loginWidget);

    // Заголовок
    QLabel *titleLabel = new QLabel("Authorization");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 20px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // Форма входа
    QWidget *formWidget = new QWidget;
    QFormLayout *formLayout = new QFormLayout(formWidget);

    QLineEdit *usernameEdit = new QLineEdit;
    usernameEdit->setPlaceholderText("Введите имя пользователя");
    QLineEdit *passwordEdit = new QLineEdit;
    passwordEdit->setPlaceholderText("Введите пароль");
    passwordEdit->setEchoMode(QLineEdit::Password);

    formLayout->addRow("Username:", usernameEdit);
    formLayout->addRow("Password:", passwordEdit);

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *loginButton = new QPushButton("Sign in");

    buttonLayout->addWidget(loginButton);
    buttonLayout->addStretch();

    layout->addWidget(titleLabel);
    layout->addWidget(formWidget);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    // Подключение сигналов
    connect(loginButton, &QPushButton::clicked, [this, usernameEdit, passwordEdit]() {
        if (validateLogin(usernameEdit->text(), passwordEdit->text())) {
            showMainCatalog();
        } else {
            QMessageBox::warning(this, "Ошибка", "Неверные данные");
        }
    });

    return loginWidget;
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
    QPushButton *logoutButton = new QPushButton("Выйти");

    controlLayout->addWidget(addButton);
    controlLayout->addStretch();
    controlLayout->addWidget(logoutButton);

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
    connect(applyFiltersBtn, &QPushButton::clicked, this, &MainWindow::searchTracks);
    connect(resetFiltersBtn, &QPushButton::clicked, this, [this]() {
        resetSearch();
        updateTrackTable();
    });
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::logout);
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

QWidget* MainWindow::createSearchScreen() {
    QWidget *searchWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(searchWidget);

    QLabel *titleLabel = new QLabel("Поиск и фильтры");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin: 10px;");

    // Группа фильтров
    QGroupBox *filterGroup = new QGroupBox("Фильтры поиска");
    QFormLayout *filterLayout = new QFormLayout(filterGroup);

    // Локальные поля, чтобы не перезаписывать указатели панели справа
    QLineEdit *localSearchTitleEdit = new QLineEdit;
    QLineEdit *localSearchArtistEdit = new QLineEdit;
    QLineEdit *localSearchAlbumEdit = new QLineEdit;
    QLineEdit *localSearchGenreEdit = new QLineEdit;

    QSpinBox *localSearchMinYear = new QSpinBox;
    localSearchMinYear->setRange(1900, 2100);
    localSearchMinYear->setSpecialValueText("Любой");
    QSpinBox *localSearchMaxYear = new QSpinBox;
    localSearchMaxYear->setRange(1900, 2100);
    localSearchMaxYear->setSpecialValueText("Любой");

    QSpinBox *localSearchMinDuration = new QSpinBox;
    localSearchMinDuration->setRange(1, 3600);
    localSearchMinDuration->setSpecialValueText("Любая");
    localSearchMinDuration->setSuffix(" сек");
    QSpinBox *localSearchMaxDuration = new QSpinBox;
    localSearchMaxDuration->setRange(1, 3600);
    localSearchMaxDuration->setSpecialValueText("Любая");
    localSearchMaxDuration->setSuffix(" сек");

    filterLayout->addRow("Название:", localSearchTitleEdit);
    filterLayout->addRow("Исполнитель:", localSearchArtistEdit);
    filterLayout->addRow("Альбом:", localSearchAlbumEdit);
    filterLayout->addRow("Жанр:", localSearchGenreEdit);
    filterLayout->addRow("Минимальный год:", localSearchMinYear);
    filterLayout->addRow("Максимальный год:", localSearchMaxYear);
    filterLayout->addRow("Минимальная длительность:", localSearchMinDuration);
    filterLayout->addRow("Максимальная длительность:", localSearchMaxDuration);

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *searchButton = new QPushButton("Поиск");
    QPushButton *resetButton = new QPushButton("Сбросить фильтры");
    QPushButton *showAllButton = new QPushButton("Показать все");
    QPushButton *backButton = new QPushButton("Назад");

    buttonLayout->addWidget(searchButton);
    buttonLayout->addWidget(resetButton);
    buttonLayout->addWidget(showAllButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(backButton);

    layout->addWidget(titleLabel);
    layout->addWidget(filterGroup);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    connect(searchButton, &QPushButton::clicked, this, &MainWindow::searchTracks);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetSearch);
    connect(showAllButton, &QPushButton::clicked, [this]() {
        updateTrackTable();
        showMainCatalog();
    });
    connect(backButton, &QPushButton::clicked, this, &MainWindow::showMainCatalog);

    return searchWidget;
}

QWidget* MainWindow::createTrackDetailsScreen() {
    QWidget *detailsWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(detailsWidget);

    QLabel *titleLabel = new QLabel("Детали трека");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin: 10px;");

    QFormLayout *formLayout = new QFormLayout;

    QLabel *idLabel = new QLabel;
    QLabel *titleLabelValue = new QLabel;
    QLabel *artistLabel = new QLabel;
    QLabel *albumLabel = new QLabel;
    QLabel *yearLabel = new QLabel;
    QLabel *genreLabel = new QLabel;
    QLabel *durationLabel = new QLabel;

    formLayout->addRow("ID:", idLabel);
    formLayout->addRow("Название:", titleLabelValue);
    formLayout->addRow("Исполнитель:", artistLabel);
    formLayout->addRow("Альбом:", albumLabel);
    formLayout->addRow("Год:", yearLabel);
    formLayout->addRow("Жанр:", genreLabel);
    formLayout->addRow("Длительность:", durationLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *backButton = new QPushButton("Назад");

    buttonLayout->addStretch();
    buttonLayout->addWidget(backButton);

    layout->addWidget(titleLabel);
    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    connect(backButton, &QPushButton::clicked, this, &MainWindow::showMainCatalog);

    return detailsWidget;
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
