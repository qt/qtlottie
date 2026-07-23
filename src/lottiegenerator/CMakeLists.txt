# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_module(LottieVectorImageGeneratorPrivate
    INTERNAL_MODULE
    SOURCES
        qlottieanimationbuilder_p.h qlottieanimationbuilder.cpp
        qlottievisitor_p.h qlottievisitor.cpp
    LIBRARIES
        Qt::Core
        Qt::QuickPrivate
        Qt::QuickShapesPrivate
        Qt::QuickVectorImageGeneratorPrivate
        Qt::LottiePrivate
        Qt::LottieVectorImageHelpers
        Qt::LottieVectorImageHelpersPrivate
        Qt::QuickTimeline
        Qt::QuickTimelinePrivate
)

if(ANDROID)
    qt_internal_force_export_module_dependency(LottieVectorImageGeneratorPrivate)
endif()
