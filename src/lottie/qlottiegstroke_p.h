// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEGSTROKE_P_H
#define QLOTTIEGSTROKE_P_H

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

#include <QtLottie/private/qlottiespatialproperty_p.h>
#include <QtLottie/private/qlottiestroke_p.h>
#include <QtLottie/private/qlottiegradientholder_p.h>

QT_BEGIN_NAMESPACE

class Q_LOTTIE_EXPORT QLottieGStroke : public QLottieGradientHolder<QLottieStroke>
{
public:
    explicit QLottieGStroke(const QLottieGStroke &other);
    QLottieGStroke(QLottieBase *parent = nullptr);

    QLottieBase *clone() const override;

    void updateProperties(int frame) override;
    void render(QLottieRenderer &renderer) const override;
    int parse(const QJsonObject &definition) override;
    QPen pen() const override;
};

QT_END_NAMESPACE

#endif // QLOTTIEGSTROKE_P_H
