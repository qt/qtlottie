// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qlottiegfill_p.h"

#include <QLinearGradient>
#include <QRadialGradient>
#include <QtMath>
#include <QColor>
#include <QString>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottieGFill::QLottieGFill(const QLottieGFill &other)
    : QLottieGradientHolder<QLottieShape>(other)
{
    if (m_hidden)
        return;

    m_opacity = other.m_opacity;
    m_fillRule = other.m_fillRule;
}

QLottieGFill::~QLottieGFill()
{
}

QLottieBase *QLottieGFill::clone() const
{
    return new QLottieGFill(*this);
}

QLottieGFill::QLottieGFill(QLottieBase *parent)
    : QLottieGradientHolder<QLottieShape>(parent)
{
}

void QLottieGFill::updateProperties(int frame)
{
    m_opacity.update(frame);
    updateGradientProperties(frame);

    qreal opacity = m_opacity.value() / 100.0;
    setGradient(opacity);
}

void QLottieGFill::render(QLottieRenderer &renderer) const
{
    renderer.render(*this);
}

int QLottieGFill::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    if (!checkRequiredKeys(definition, "Gradient"_L1, { "o"_L1 }, m_name))
        return -1;

    QJsonObject opacity = definition.value("o"_L1).toObject();
    opacity = resolveExpression(opacity);
    m_opacity.construct(opacity);

    qCDebug(lcLottieQtLottieParser) << "QLottieGFill::parse():" << m_name;

    if (!parseGradientProperties(definition))
        return -1;

    const int fillValue = definition.value("r"_L1).toInt();
    m_fillRule = (fillValue == 2) ? Qt::OddEvenFill : Qt::WindingFill;

    return 0;
}

Qt::FillRule QLottieGFill::fillRule() const
{
    return m_fillRule;
}

QT_END_NAMESPACE
