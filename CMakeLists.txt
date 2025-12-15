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
        src/mainwindow.cpp
        includes/mainwindow.h
        mainwindow.ui
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(NewMusicCatalog
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        includes/FileManager.h
        includes/MP3FileManager.h src/MP3FileManager.cpp
        includes/MusicCatalog.h src/MusicCatalog.cpp
        includes/Track.h src/Track.cpp
        includes/SearchResultsWindow.h src/SearchResultsWindow.cpp
        includes/TrackRepository.h src/TrackRepository.cpp
        includes/TrackSearcher.h src/TrackSearcher.cpp
        includes/TrackSearchParams.h
        includes/TrackSorter.h src/TrackSorter.cpp
        includes/MP3MetadataReader.h src/MP3MetadataReader.cpp
        includes/MP3FileNameParser.h src/MP3FileNameParser.cpp
        includes/MP3FileOperations.h src/MP3FileOperations.cpp
        includes/MP3FileFinder.h src/MP3FileFinder.cpp
        includes/TXTParser.h src/TXTParser.cpp
        includes/TXTWriter.h src/TXTWriter.cpp
        includes/TXTReader.h src/TXTReader.cpp
        includes/MusicCatalogException.h src/MusicCatalogException.cpp
        includes/FileException.h src/FileException.cpp
        includes/TrackException.h src/TrackException.cpp
        includes/ValidationException.h src/ValidationException.cpp
        includes/ParseException.h src/ParseException.cpp
        includes/GenreManager.h src/GenreManager.cpp
        includes/ReportGenerator.h src/ReportGenerator.cpp
        includes/YandexMusicAPI.h src/YandexMusicAPI.cpp
        includes/YandexMusicIntegrator.h src/YandexMusicIntegrator.cpp
        includes/TrackTableHighlighter.h src/TrackTableHighlighter.cpp
        includes/TemplateUtils.h
        includes/Parser.h
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
