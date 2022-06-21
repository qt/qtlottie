/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the lottie-qt module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef BMSHAPETRANSFORM_P_H
#define BMSHAPETRANSFORM_P_H

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

#include <QtBodymovin/private/bmshape_p.h>
#include <QtBodymovin/private/bmbasictransform_p.h>
#include <QtBodymovin/private/bmproperty_p.h>

QT_BEGIN_NAMESPACE

class QJsonObject;

class BODYMOVIN_EXPORT BMShapeTransform :  public BMBasicTransform
{
public:
    explicit BMShapeTransform(const BMShapeTransform &other);
    BMShapeTransform(const QJsonObject &definition, const QVersionNumber &version, BMBase *parent);

    BMBase *clone() const override;

    void construct(const QJsonObject &definition, const QVersionNumber &version);

    void updateProperties(int frame) override;
    void render(LottieRenderer &renderer) const override;

    qreal skew() const;
    qreal skewAxis() const;
    qreal shearX() const;
    qreal shearY() const;
    qreal shearAngle() const;

protected:
    BMProperty<qreal> m_skew;
    BMProperty<qreal> m_skewAxis;
    qreal m_shearX;
    qreal m_shearY;
    qreal m_shearAngle;
};

QT_END_NAMESPACE

#endif // BMSHAPETRANSFORM_P_H
