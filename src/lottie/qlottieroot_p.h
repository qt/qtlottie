// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEROOT_P_H
#define QLOTTIEROOT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtLottie/private/qlottiebase_p.h>

QT_BEGIN_NAMESPACE

class LOTTIE_EXPORT QLottieRoot : public QLottieBase
{
public:
    QLottieRoot() = default;
    explicit QLottieRoot(const QLottieRoot &other);

    QLottieBase *clone() const override;

    int parseSource(const QByteArray &jsonSource,
                    const QUrl &fileSource,
                    QVersionNumber version = QVersionNumber());
};

QT_END_NAMESPACE

#endif // QLOTTIEROOT_P_H
