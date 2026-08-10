# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(parse LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Qml Quick)
find_package(Qt6 COMPONENTS LottiePrivate QuickVectorImagePrivate QuickPrivate)

qt_add_executable(vectorimage
    main.cpp
)

target_link_libraries(vectorimage PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::LottiePrivate
    Qt::QuickVectorImagePrivate
    Qt::QuickPrivate
)
target_link_libraries(vectorimage PRIVATE
     -fsanitize=fuzzer
)

