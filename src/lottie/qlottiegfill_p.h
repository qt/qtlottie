// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QLOTTIEGFILL_P_H
#define QLOTTIEGFILL_P_H

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

#include <QVector4D>
#include <QGradient>

#include <QtLottie/private/qlottiegroup_p.h>
#include <QtLottie/private/qlottieproperty_p.h>
#include <QtLottie/private/qlottieproperty_p.h>
#include <QtLottie/private/qlottiespatialproperty_p.h>
#include <QtLottie/private/qlottiegradientholder_p.h>

QT_BEGIN_NAMESPACE

class Q_LOTTIE_EXPORT QLottieGFill : public QLottieGradientHolder<QLottieShape>
{
public:
    explicit QLottieGFill(const QLottieGFill &other);
    QLottieGFill(QLottieBase *parent = nullptr);
    ~QLottieGFill() override;

    QLottieBase *clone() const override;

    void updateProperties(int frame) override;
    void render(QLottieRenderer &renderer) const override;
    int parse(const QJsonObject &definition) override;
    Qt::FillRule fillRule() const;
    qreal opacity() const;

protected:
    Qt::FillRule m_fillRule = Qt::WindingFill;
    QLottieProperty<qreal> m_opacity;
};

QT_END_NAMESPACE

#endif // QLOTTIEGFILL_P_H
