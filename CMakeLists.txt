cmake_minimum_required(VERSION 3.16)

project(NewMusicCatalog VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
        src/main.cpp
        src/ui/mainwindow.cpp
        includes/ui/mainwindow.h
        mainwindow.ui
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(NewMusicCatalog
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        # Core
        includes/core/Track.h src/core/Track.cpp
        includes/core/TrackRepository.h src/core/TrackRepository.cpp
        includes/core/MusicCatalog.h src/core/MusicCatalog.cpp
        includes/core/TrackSearcher.h src/core/TrackSearcher.cpp
        includes/core/TrackSearchParams.h
        includes/core/TrackSorter.h src/core/TrackSorter.cpp
        includes/core/GenreManager.h src/core/GenreManager.cpp
        # Exceptions
        includes/exceptions/MusicCatalogException.h src/exceptions/MusicCatalogException.cpp
        includes/exceptions/TrackException.h src/exceptions/TrackException.cpp
        includes/exceptions/FileException.h src/exceptions/FileException.cpp
        includes/exceptions/ValidationException.h src/exceptions/ValidationException.cpp
        includes/exceptions/ParseException.h src/exceptions/ParseException.cpp
        # File Operations
        includes/file_operations/FileManager.h
        includes/file_operations/TXTReader.h src/file_operations/TXTReader.cpp
        includes/file_operations/TXTWriter.h src/file_operations/TXTWriter.cpp
        includes/file_operations/TXTParser.h src/file_operations/TXTParser.cpp
        includes/file_operations/Parser.h
        includes/file_operations/TemplateUtils.h
        # MP3
        includes/mp3/MP3FileManager.h src/mp3/MP3FileManager.cpp
        includes/mp3/MP3MetadataReader.h src/mp3/MP3MetadataReader.cpp
        includes/mp3/MP3FileNameParser.h src/mp3/MP3FileNameParser.cpp
        includes/mp3/MP3FileOperations.h src/mp3/MP3FileOperations.cpp
        includes/mp3/MP3FileFinder.h src/mp3/MP3FileFinder.cpp
        # Integration
        includes/integration/YandexMusicAPI.h src/integration/YandexMusicAPI.cpp
        includes/integration/YandexMusicIntegrator.h src/integration/YandexMusicIntegrator.cpp
        # UI
        includes/ui/SearchResultsWindow.h src/ui/SearchResultsWindow.cpp
        includes/ui/TrackTableHighlighter.h src/ui/TrackTableHighlighter.cpp
        includes/ui/ReportGenerator.h src/ui/ReportGenerator.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET NewMusicCatalog APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(NewMusicCatalog SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(NewMusicCatalog
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(NewMusicCatalog PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.NewMusicCatalog)
endif()
set_target_properties(NewMusicCatalog PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS NewMusicCatalog
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(NewMusicCatalog)
endif()
