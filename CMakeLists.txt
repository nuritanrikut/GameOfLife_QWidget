cmake_minimum_required(VERSION 3.5)

project(game_of_life_qwidget LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp
        MainWindow.hpp
        GameWidget.cpp
        GameWidget.hpp
)

qt_add_executable(game_of_life_qwidget ${PROJECT_SOURCES})

target_link_libraries(game_of_life_qwidget PRIVATE Qt6::Widgets pthread tbb)


