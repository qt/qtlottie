/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/

#include "bmfill_p.h"

QT_BEGIN_NAMESPACE

BMFill::BMFill(const BMFill &other)
    : BMShape(other)
{
    m_color = other.m_color;
    m_opacity = other.m_opacity;
}

BMFill::BMFill(const QJsonObject &definition, BMBase *parent)
{
    setParent(parent);
    BMBase::parse(definition);
    if (m_hidden)
        return;

    qCDebug(lcLottieQtBodymovinParser) << "BMFill::construct():" << m_name;

    QJsonObject color = definition.value(QLatin1String("c")).toObject();
    m_color.construct(color);

    QJsonObject opacity = definition.value(QLatin1String("o")).toObject();
    opacity = resolveExpression(opacity);
    m_opacity.construct(opacity);
}

BMBase *BMFill::clone() const
{
    return new BMFill(*this);
}

void BMFill::updateProperties(int frame)
{
    m_color.update(frame);
    m_opacity.update(frame);
}

void BMFill::render(LottieRenderer &renderer) const
{
    renderer.render(*this);
}

QColor BMFill::color() const
{
    QVector4D cVec = m_color.value();
    QColor color;
    qreal r = static_cast<qreal>(cVec.x());
    qreal g = static_cast<qreal>(cVec.y());
    qreal b = static_cast<qreal>(cVec.z());
    qreal a = static_cast<qreal>(cVec.w());
    color.setRgbF(r, g, b, a);
    return color;
}

qreal BMFill::opacity() const
{
    return m_opacity.value();
}

QT_END_NAMESPACE
