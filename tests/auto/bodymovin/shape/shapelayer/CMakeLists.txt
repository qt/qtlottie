# Collect test data
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    data/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_shapelayer
    SOURCES
        tst_bmshapelayer.cpp
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::Test
        Qt::BodymovinPrivate
    TESTDATA ${test_data}
)
