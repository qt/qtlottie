// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEGLOBAL_H
#define QLOTTIEGLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LOTTIE_LIBRARY)
#  define LOTTIE_EXPORT Q_DECL_EXPORT
#else
#  define LOTTIE_EXPORT Q_DECL_IMPORT
#endif

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

#endif // QLOTTIEGLOBAL_H
