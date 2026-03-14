// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qlottieprecomplayer_p.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QLoggingCategory>
#include <QString>

#include "qlottiebasictransform_p.h"
#include "qlottieprecomposition_p.h"
#include "qlottierenderer_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottiePrecompLayer::QLottiePrecompLayer(const QLottiePrecompLayer &other)
    : QLottieLayer(other)
{
    m_refId = other.m_refId;
}

QLottieBase *QLottiePrecompLayer::clone() const
{
    return new QLottiePrecompLayer(*this);
}

void QLottiePrecompLayer::render(QLottieRenderer &renderer) const
{
    if (!m_isActive)
        return;

    renderer.saveState();

    QLottieLayer::render(renderer);

    renderer.finish(*this);
    renderer.restoreState();
}

int QLottiePrecompLayer::parse(const QJsonObject &definition)
{
    m_type = LOTTIE_LAYER_PRECOMP_IX;

    QLottieLayer::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottiePrecompLayer::parse()" << m_name;

    m_startTime = definition.value("st"_L1).toDouble(); // only relevant for precomps

    if (!checkRequiredKeys(definition, "Precomp Layer"_L1, { "refId"_L1 }, m_name))
        return -1;
    m_refId = definition.value("refId"_L1).toString();

    m_size = QSize(definition.value("w"_L1).toInt(-1), definition.value("h"_L1).toInt(-1));

    qCDebug(lcLottieQtLottieParser) << "QLottiePrecompLayer created for refId" << m_refId;
    return 0;
}

QT_END_NAMESPACE
