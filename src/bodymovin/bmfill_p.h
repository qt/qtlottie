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

#ifndef BMFILL_P_H
#define BMFILL_P_H

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

#include <QColor>
#include <QVector4D>

#include <QtBodymovin/private/bmgroup_p.h>
#include <QtBodymovin/private/bmproperty_p.h>

QT_BEGIN_NAMESPACE

class BODYMOVIN_EXPORT BMFill : public BMShape
{
public:
    BMFill() = default;
    explicit BMFill(const BMFill &other);
    BMFill(const QJsonObject &definition, BMBase *parent = nullptr);

    BMBase *clone() const override;

    void updateProperties(int frame) override;

    void render(LottieRenderer &renderer) const override;

    QColor color() const;
    qreal opacity() const;

protected:
    BMProperty4D<QVector4D> m_color;
    BMProperty<qreal> m_opacity;
};

QT_END_NAMESPACE

#endif // BMFILL_P_H
