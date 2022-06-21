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

#ifndef BMSTROKE_P_H
#define BMSTROKE_P_H

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

#include <QPen>
#include <QVector4D>

#include <QtBodymovin/private/bmshape_p.h>
#include <QtBodymovin/private/bmproperty_p.h>

QT_BEGIN_NAMESPACE

class BODYMOVIN_EXPORT BMStroke : public BMShape
{
public:
    BMStroke() = default;
    explicit BMStroke(const BMStroke &other);
    BMStroke(const QJsonObject &definition, const QVersionNumber &version,
             BMBase *parent = nullptr);

    BMBase *clone() const override;

    void updateProperties(int frame) override;
    void render(LottieRenderer &renderer) const override;

    QPen pen() const;
    qreal opacity() const;

protected:
    QColor getColor() const;

protected:
    BMProperty<qreal> m_opacity;
    BMProperty<qreal> m_width;
    BMProperty4D<QVector4D> m_color;
    Qt::PenCapStyle m_capStyle;
    Qt::PenJoinStyle m_joinStyle;
    qreal m_miterLimit;
};

QT_END_NAMESPACE

#endif // BMSTROKE_P_H
