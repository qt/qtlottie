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
#include "qlottieroot_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottiePrecompLayer::QLottiePrecompLayer(const QLottiePrecompLayer &other)
    : QLottieLayer(other)
{
    m_refId = other.m_refId;
    m_timeRemap = other.m_timeRemap;
}

QLottieBase *QLottiePrecompLayer::clone() const
{
    return new QLottiePrecompLayer(*this);
}

void QLottiePrecompLayer::updateProperties(int frame)
{
    if (m_hasLinkedLayer)
        resolveLinkedLayer();

    m_isActive = active(frame);
    if (!m_isActive)
        return;

    // Update first effects, as they are not children of the layer
    if (m_effects) {
        for (QLottieBase* effect : m_effects->children())
            effect->updateProperties(frame);
    }

    if (m_layerTransform)
        m_layerTransform->updateProperties(frame);

    int adjFrame = frame;
    if (m_timeRemap.isAnimated()) {
        m_timeRemap.update(frame);
        int frameRate = 30;
        const QLottieBase *root = topRoot();
        if (Q_LIKELY(root && root->type() == LOTTIE_ROOT_IX))
            frameRate = static_cast<const QLottieRoot *>(root)->frameRate();
        adjFrame = qRound(m_timeRemap.value() * frameRate);
    } else if (m_startTime || m_stretch) {
        if (m_stretch)
            adjFrame = qRound((frame - m_startTime) / m_stretch);
        else
            adjFrame = qRound(frame - m_startTime);
    }

    QLottieBase::updateProperties(adjFrame);
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

    if (!checkRequiredKeys(definition, "Precomp Layer"_L1, { "refId"_L1 }, m_name))
        return -1;
    m_refId = definition.value("refId"_L1).toString();

    m_startTime = definition.value("st"_L1).toDouble(); // only relevant for precomp layers
    m_stretch = definition.value("sr"_L1).toDouble(); // only relevant for precomp layers

    QJsonObject timeRemap = definition.value("tm"_L1).toObject();
    timeRemap = resolveExpression(timeRemap);
    m_timeRemap.construct(timeRemap);

    m_size = QSize(definition.value("w"_L1).toInt(-1), definition.value("h"_L1).toInt(-1));

    qCDebug(lcLottieQtLottieParser) << "QLottiePrecompLayer created for refId" << m_refId;
    return 0;
}

QT_END_NAMESPACE
