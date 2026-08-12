// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottiegradientholder_p.h"

QT_BEGIN_NAMESPACE

QGradient::Type QLottieGradientHolderContent::gradientType() const
{
    if (m_gradient)
        return m_gradient->type();
    else
        return QGradient::NoGradient;
}

void QLottieGradientHolderContent::updateGradientProperties(int frame)
{
    QGradient::Type type = gradientType();
    if (type != QGradient::LinearGradient && type != QGradient::RadialGradient)
        return;

    m_startPoint.update(frame);
    m_endPoint.update(frame);
    m_highlightLength.update(frame);
    m_highlightAngle.update(frame);
    QHash<qreal, QLottieProperty4D<QVector4D>>::iterator colorIt = m_colors.begin();
    while (colorIt != m_colors.end()) {
        (*colorIt).update(frame);
        ++colorIt;
    }
}

QPointF QLottieGradientHolderContent::startPoint() const
{
    return m_startPoint.value();
}

QPointF QLottieGradientHolderContent::endPoint() const
{
    return m_endPoint.value();
}

qreal QLottieGradientHolderContent::highlightLength() const
{
    return m_highlightLength.value();
}

qreal QLottieGradientHolderContent::highlightAngle() const
{
    return m_highlightAngle.value();
}

QGradient *QLottieGradientHolderContent::value() const
{
    return m_gradient;
}

void QLottieGradientHolderContent::setGradient(qreal opacity)
{
    if (!m_gradient)
        return;

    QHash<qreal, QLottieProperty4D<QVector4D>>::iterator colorIt = m_colors.begin();
    while (colorIt != m_colors.end()) {
        QVector4D colorPos = (*colorIt).value();
        qreal pos = colorIt.key();
        QColor color;
        color.setRedF(static_cast<qreal>(colorPos[0]));
        color.setGreenF(static_cast<qreal>(colorPos[1]));
        color.setBlueF(static_cast<qreal>(colorPos[2]));

        qreal o = static_cast<qreal>(colorPos[3]) * opacity;
        color.setAlphaF(o);
        m_gradient->setColorAt(pos, color);
        ++colorIt;
    }

    switch (gradientType()) {
    case QGradient::LinearGradient:
    {
        QLinearGradient *g = static_cast<QLinearGradient*>(m_gradient);
        g->setStart(m_startPoint.value());
        g->setFinalStop(m_endPoint.value());
        break;
    }
    case QGradient::RadialGradient:
    {
        QRadialGradient *g = static_cast<QRadialGradient*>(m_gradient);
        QLineF radLine(m_startPoint.value(), m_endPoint.value());
        g->setCenter(radLine.p1());
        g->setRadius(radLine.length());
        radLine.setAngle(radLine.angle() - m_highlightAngle.value());
        // QRadialGradient needs focalPoint to be inside (and not on) radius circle
        qreal radFraction = qMin((m_highlightLength.value() / 100.0), 0.999);
        g->setFocalPoint(radLine.pointAt(radFraction));
        break;
    }
    default:
        break;
    }
}

QT_END_NAMESPACE
