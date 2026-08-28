# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_lottie_qml LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_lottie_qml
    GUI
    QMLTEST
    SOURCES
        tst_qml.cpp
    LIBRARIES
        Qt::Quick
    TESTDATA
        data/tst_main.qml
)

qt_internal_add_resource(tst_lottie_qml "qml"
    PREFIX
        "/"
    FILES
        data/rec_pos_col_opa.json
)
