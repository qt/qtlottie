# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(TARGET Qt::QuickPrivate
    AND TARGET Qt::QuickVectorImagePrivate
    AND TARGET Qt::QuickVectorImageGeneratorPrivate
    AND TARGET Qt::LottieVectorImageGeneratorPrivate)
    add_subdirectory(lottietoqml)
endif()
