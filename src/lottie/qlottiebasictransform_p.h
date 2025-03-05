// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEBASICTRANSFORM_P_H
#define QLOTTIEBASICTRANSFORM_P_H

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

#include <QPointF>

#include <QtLottie/private/qlottieshape_p.h>
#include <QtLottie/private/qlottieproperty_p.h>
#include <QtLottie/private/qlottiespatialproperty_p.h>

QT_BEGIN_NAMESPACE

class QJsonObject;

class LOTTIE_EXPORT QLottieBasicTransform : public QLottieShape
{
public:
    QLottieBasicTransform() = default;
    explicit QLottieBasicTransform(const QLottieBasicTransform &other);
    QLottieBasicTransform(const QJsonObject &definition, const QVersionNumber &version,
                     QLottieBase *parent = nullptr);

    QLottieBase *clone() const override;

    void construct(const QJsonObject &definition, const QVersionNumber &version);

    void updateProperties(int frame) override;
    void render(QLottieRenderer &renderer) const override;

    QPointF anchorPoint() const;
    virtual QPointF position() const;
    QPointF scale() const;
    qreal rotation() const;
    qreal opacity() const;

protected:
    QLottieSpatialProperty m_anchorPoint;
    bool m_splitPosition = false;
    QLottieSpatialProperty m_position;
    QLottieProperty<qreal> m_xPos;
    QLottieProperty<qreal> m_yPos;
    QLottieProperty2D<QPointF> m_scale;
    QLottieProperty<qreal> m_rotation;
    QLottieProperty<qreal> m_opacity;
};

QT_END_NAMESPACE

#endif // QLOTTIEBASICTRANSFORM_P_H
