// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottiegstroke_p.h"

QT_BEGIN_NAMESPACE

QLottieGStroke::QLottieGStroke(const QLottieGStroke &other)
    : QLottieGradientHolder<QLottieStroke>(other)
{
}

QLottieGStroke::QLottieGStroke(QLottieBase *parent)
    : QLottieGradientHolder<QLottieStroke>(parent)
{
}

QLottieBase *QLottieGStroke::clone() const
{
    return new QLottieGStroke(*this);
}

void QLottieGStroke::updateProperties(int frame)
{
    QLottieStroke::updateProperties(frame);
    updateGradientProperties(frame);

    setGradient(opacity() / 100.0);
}

void QLottieGStroke::render(QLottieRenderer &renderer) const
{
    renderer.render(*this);
}

int QLottieGStroke::parse(const QJsonObject &definition)
{
    if (QLottieStroke::parse(definition) < 0)
        return -1;

    if (!parseGradientProperties(definition))
        return -1;

    return 0;
}

QPen QLottieGStroke::pen() const
{
    QPen p = QLottieStroke::pen();
    if (p.style() == Qt::NoPen)
        return p;

    if (QGradient *gradient = value())
        p.setBrush(*gradient);
    return p;
}

QT_END_NAMESPACE
