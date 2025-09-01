// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottieprecomplayer_p.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QLoggingCategory>
#include <QString>

#include "qlottiebasictransform_p.h"
#include "qlottierenderer_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottiePrecompLayer::QLottiePrecompLayer(const QLottiePrecompLayer &other)
    : QLottieLayer(other)
{
}

QLottiePrecompLayer::QLottiePrecompLayer(const QMap<QString, QJsonObject> &assets)
    : m_assets(assets)
{
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

    qCDebug(lcLottieQtLottieParser) << "QLottiePrecompLayer::QLottiePrecompLayer()" << m_name;

    m_startTime = definition.value(u"st"_s).toDouble(); // only relevant for precomps

    if (!checkRequiredKey(definition, u"Precomp Layer"_s, {u"refId"_s}, m_name))
        return -1;
    QString refId = definition.value(u"refId"_s).toString();

    QJsonObject asset = m_assets.value(refId);
    QJsonArray jsonLayers = asset.value(u"layers"_s).toArray();
    int numLayers = QLottieLayer::constructLayers(jsonLayers, this, m_assets);

    m_size = QSize(definition.value(u"w"_s).toInt(-1), definition.value(u"h"_s).toInt(-1));

    qCDebug(lcLottieQtLottieParser) << "QLottiePrecompLayer created" << numLayers << "layers";
    return 0;
}

QT_END_NAMESPACE
