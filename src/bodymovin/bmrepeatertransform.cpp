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

#include "bmrepeatertransform_p.h"

QT_BEGIN_NAMESPACE

BMRepeaterTransform::BMRepeaterTransform(const BMRepeaterTransform &other)
    : BMBasicTransform(other)
{
    m_startOpacity = other.m_startOpacity;
    m_endOpacity = other.m_endOpacity;
    m_opacities = other.m_opacities;
}

BMRepeaterTransform::BMRepeaterTransform(const QJsonObject &definition,
                                         const QVersionNumber &version, BMBase *parent)
{
    setParent(parent);
    construct(definition, version);
}

BMBase *BMRepeaterTransform::clone() const
{
    return new BMRepeaterTransform(*this);
}

void BMRepeaterTransform::construct(const QJsonObject &definition, const QVersionNumber &version)
{
    qCDebug(lcLottieQtBodymovinParser) << "BMRepeaterTransform::construct():" << name();

    BMBasicTransform::construct(definition, version);
    if (m_hidden)
        return;

    QJsonObject startOpacity = definition.value(QLatin1String("so")).toObject();
    startOpacity = resolveExpression(startOpacity);
    m_startOpacity.construct(startOpacity, version);

    QJsonObject endOpacity = definition.value(QLatin1String("eo")).toObject();
    endOpacity = resolveExpression(endOpacity);
    m_endOpacity.construct(endOpacity, version);
}

void BMRepeaterTransform::updateProperties(int frame)
{
    BMBasicTransform::updateProperties(frame);

    m_startOpacity.update(frame);
    m_endOpacity.update(frame);

    m_opacities.clear();
    for (int i = 0; i < m_copies; i++) {
        qreal opacity = m_startOpacity.value() +
                (m_endOpacity.value() - m_startOpacity.value()) * i / m_copies;
        m_opacities.push_back(opacity);
    }
}

void BMRepeaterTransform::render(LottieRenderer &renderer) const
{
    renderer.render(*this);
}

void BMRepeaterTransform::setInstanceCount(int copies)
{
    m_copies = copies;
}

qreal BMRepeaterTransform::opacityAtInstance(int instance) const
{
    return m_opacities.at(instance) / 100.0;
}

qreal BMRepeaterTransform::startOpacity() const
{
    return m_startOpacity.value();
}

qreal BMRepeaterTransform::endOpacity() const
{
    return m_endOpacity.value();
}

QT_END_NAMESPACE
