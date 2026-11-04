# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(parseSource LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS Core Gui)
find_package(Qt6 COMPONENTS LottiePrivate)

qt_add_executable(parseSource
    main.cpp
)

target_link_libraries(parseSource PUBLIC
    Qt::Core
    Qt::Gui
    Qt::LottiePrivate
)
if(DEFINED ENV{LIB_FUZZING_ENGINE})
    target_link_libraries(parseSource PRIVATE
        $ENV{LIB_FUZZING_ENGINE}
    )
else()
    target_link_libraries(parseSource PRIVATE
        -fsanitize=fuzzer
    )
endif()

qt_import_plugins(parseSource
    INCLUDE Qt::QMinimalIntegrationPlugin
)
