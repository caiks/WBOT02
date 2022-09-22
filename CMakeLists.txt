cmake_minimum_required(VERSION 3.5)

project(WBOT02 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Gui Multimedia MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Gui Multimedia MultimediaWidgets)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 7)
qt_standard_project_setup()
else()
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
if (${QT_VERSION_MINOR} GREATER_EQUAL 3)
qt_standard_project_setup()
endif()
endif()
endif()

find_package (Threads)

add_subdirectory(../AlignmentActive ../AlignmentActive_build)

include_directories(../rapidjson/include ../AlignmentC ../AlignmentRepaC ../AlignmentActive)

set(PROJECT_SOURCES
        dev.cpp
        dev.h
        main.cpp
        actor.cpp
        actor.h
        win001.cpp
        win001.h
        win001.ui
        win002.cpp
        win002.h
        win002.ui        
        win003.cpp
        win003.h
        win003.ui
        win004.cpp
        win004.h
        win004.ui
        win005.cpp
        win005.h
        win005.ui
        win006.cpp
        win006.h
        win006.ui
        win007.cpp
        win007.h
        win007.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WBOT02
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET WBOT02 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(WBOT02 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(WBOT02
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(WBOT02 PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    AlignmentActive
    AlignmentRepaC
    AlignmentC
    Threads::Threads)

set_target_properties(WBOT02 PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WBOT02)
endif()
