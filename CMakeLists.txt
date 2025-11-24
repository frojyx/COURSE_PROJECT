cmake_minimum_required(VERSION 3.16)

project(NewMusicCatalog VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(NewMusicCatalog
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        FileManager.h
        MP3FileManager.h MP3FileManager.cpp
        MusicCatalog.h MusicCatalog.cpp
        Track.h Track.cpp
        SearchResultsWindow.h SearchResultsWindow.cpp
        TrackRepository.h TrackRepository.cpp
        TrackSearcher.h TrackSearcher.cpp
        TrackSorter.h TrackSorter.cpp
        MP3MetadataReader.h MP3MetadataReader.cpp
        MP3FileNameParser.h MP3FileNameParser.cpp
        MP3FileOperations.h MP3FileOperations.cpp
        MP3FileFinder.h MP3FileFinder.cpp
        TXTParser.h TXTParser.cpp
        TXTWriter.h TXTWriter.cpp
        TXTReader.h TXTReader.cpp
        MusicCatalogException.h MusicCatalogException.cpp
        FileException.h FileException.cpp
        TrackException.h TrackException.cpp
        ValidationException.h ValidationException.cpp
        ParseException.h ParseException.cpp
        GenreManager.h GenreManager.cpp
        ReportGenerator.h ReportGenerator.cpp
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

target_link_libraries(NewMusicCatalog PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

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
