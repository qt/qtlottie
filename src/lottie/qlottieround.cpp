// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottieround_p.h"

#include <QJsonObject>

#include "qlottietrimpath_p.h"

QT_BEGIN_NAMESPACE

QLottieRound::QLottieRound(const QLottieRound &other)
    : QLottieShape(other)
{
    m_position = other.m_position;
    m_radius = other.m_radius;
}

QLottieRound::QLottieRound(const QJsonObject &definition, const QVersionNumber &version, QLottieBase *parent)
{
    setParent(parent);
    construct(definition, version);
}

QLottieBase *QLottieRound::clone() const
{
    return new QLottieRound(*this);
}

void QLottieRound::construct(const QJsonObject &definition, const QVersionNumber &version)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return;

    qCDebug(lcLottieQtLottieParser) << "QLottieRound::construct():" << m_name;

    QJsonObject position = definition.value(QLatin1String("p")).toObject();
    position = resolveExpression(position);
    m_position.construct(position, version);

    QJsonObject radius = definition.value(QLatin1String("r")).toObject();
    radius = resolveExpression(radius);
    m_radius.construct(radius, version);
}

void QLottieRound::updateProperties(int frame)
{
    m_position.update(frame);
    m_radius.update(frame);

    // AE uses center of a shape as it's position,
    // in Qt a translation is needed
    QPointF center = QPointF(m_position.value().x() - m_radius.value() / 2,
                             m_position.value().y() - m_radius.value() / 2);

    m_path = QPainterPath();
    m_path.arcMoveTo(QRectF(center,
                            QSizeF(m_radius.value(), m_radius.value())), 90);
    m_path.arcTo(QRectF(center,
                        QSizeF(m_radius.value(), m_radius.value())), 90, -360);

    if (hasReversedDirection())
        m_path = m_path.toReversed();
}

void QLottieRound::render(QLottieRenderer &renderer) const
{
    renderer.render(*this);
}

bool QLottieRound::acceptsTrim() const
{
    return true;
}

QPointF QLottieRound::position() const
{
    return m_position.value();
}

qreal QLottieRound::radius() const
{
    return m_radius.value();
}

QT_END_NAMESPACE
