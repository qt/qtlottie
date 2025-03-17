// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottieprecomplayer_p.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QLoggingCategory>

#include "qlottiebasictransform_p.h"
#include "qlottierenderer_p.h"

QT_BEGIN_NAMESPACE

QLottiePrecompLayer::QLottiePrecompLayer(const QLottiePrecompLayer &other)
    : QLottieLayer(other)
{
}

QLottiePrecompLayer::QLottiePrecompLayer(const QJsonObject &definition, const QMap<QString, QJsonObject> &assets, const QVersionNumber &version)
{
    m_type = LOTTIE_LAYER_PRECOMP_IX;
    m_version = version;

    QLottieLayer::parse(definition);
    if (m_hidden)
        return;

    qCDebug(lcLottieQtLottieParser) << "QLottiePrecompLayer::QLottiePrecompLayer()" << m_name;

    QString refId = definition.value(QLatin1String("refId")).toString();
    QJsonObject asset = assets.value(refId);
    QJsonArray jsonLayers = asset.value(QLatin1String("layers")).toArray();
    int numLayers = QLottieLayer::constructLayers(jsonLayers, this, assets, version);

    qCDebug(lcLottieQtLottieParser) << "QLottiePrecompLayer created" << numLayers << "layers";
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

    renderer.restoreState();
}

QT_END_NAMESPACE
