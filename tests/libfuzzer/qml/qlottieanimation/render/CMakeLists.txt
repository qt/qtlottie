# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(render LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui)
find_package(Qt6 COMPONENTS LottiePrivate)

qt_add_executable(render
    main.cpp
)

target_link_libraries(render PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::LottiePrivate
)
if(DEFINED ENV{LIB_FUZZING_ENGINE})
    target_link_libraries(render PRIVATE
        $ENV{LIB_FUZZING_ENGINE}
    )
else()
    target_link_libraries(render PRIVATE
        -fsanitize=fuzzer
    )
endif()

qt_import_plugins(render
    INCLUDE Qt::QMinimalIntegrationPlugin
)
