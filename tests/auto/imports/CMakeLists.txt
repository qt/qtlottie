# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qt_internal_add_test(tst_imports
    GUI
    QMLTEST
    SOURCES
        tst_imports.cpp
    LIBRARIES
        Qt::Quick
    TESTDATA
        data/tst_main.qml
)

qt_internal_add_resource(tst_imports "imports"
    PREFIX
        "/"
    FILES
        data/rec_pos_col_opa.json
)
