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

#include "bmrepeater_p.h"

BMRepeater::BMRepeater(const QJsonObject &definition, const QVersionNumber &version, BMBase *parent)
{
    setParent(parent);
    m_transform.setParent(this);
    construct(definition, version);
}

BMBase *BMRepeater::clone() const
{
    return new BMRepeater(*this);
}

void BMRepeater::construct(const QJsonObject &definition, const QVersionNumber &version)
{
    qCDebug(lcLottieQtBodymovinParser) << "BMRepeater::construct():" << m_name;

    BMBase::parse(definition);
    if (m_hidden)
        return;

    QJsonObject copies = definition.value(QLatin1String("c")).toObject();
    copies = resolveExpression(copies);
    m_copies.construct(copies, version);

    QJsonObject offset = definition.value(QLatin1String("o")).toObject();
    offset = resolveExpression(offset);
    m_offset.construct(offset, version);

    m_transform.construct(definition.value(QLatin1String("tr")).toObject(), version);
}

void BMRepeater::updateProperties(int frame)
{
    m_copies.update(frame);
    m_offset.update(frame);
    m_transform.setInstanceCount(m_copies.value());
    m_transform.updateProperties(frame);
}

void BMRepeater::render(LottieRenderer &renderer) const
{
    renderer.render(*this);
}

int BMRepeater::copies() const
{
    return m_copies.value();
}

qreal BMRepeater::offset() const
{
    return m_offset.value();
}

const BMRepeaterTransform &BMRepeater::transform() const
{
    return m_transform;
}
