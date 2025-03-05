// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEIMAGE_P_H
#define QLOTTIEIMAGE_P_H

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

#include <QImage>
#include <QPointF>

#include <QtLottie/private/qlottieproperty_p.h>
#include <QtLottie/private/qlottiespatialproperty_p.h>

QT_BEGIN_NAMESPACE

class QJsonObject;

class LOTTIE_EXPORT QLottieImage : public QLottieBase
{
public:
    QLottieImage() = default;
    explicit QLottieImage(const QLottieImage &other);
    QLottieImage(const QJsonObject &definition, const QVersionNumber &version, QLottieBase *parent = nullptr);

    QLottieBase *clone() const override;

    void construct(const QJsonObject &definition, const QVersionNumber &version);

    void updateProperties(int frame) override;
    void render(QLottieRenderer &renderer) const override;

    QPointF position() const;
    qreal radius() const;

    QPointF getCenter() const { return m_center; }
    QImage getImage() const { return m_image; }

protected:
    QLottieSpatialProperty m_position;
    QLottieProperty<qreal> m_radius;

    QImage m_image;
    QPointF m_center;
};

QT_END_NAMESPACE

#endif // QLOTTIEIMAGE_P_H
