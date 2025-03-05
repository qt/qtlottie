// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottielayer_p.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QLoggingCategory>

#include "qlottieimagelayer_p.h"
#include "qlottieshapelayer_p.h"
#include "qlottiefilleffect_p.h"
#include "qlottiebasictransform_p.h"

QT_BEGIN_NAMESPACE

QLottieLayer::QLottieLayer(const QLottieLayer &other)
    : QLottieBase(other)
{
    m_layerIndex = other.m_layerIndex;
    m_startFrame = other.m_startFrame;
    m_endFrame = other.m_endFrame;
    m_startTime = other.m_startTime;
    m_blendMode = other.m_blendMode;
    m_3dLayer = other.m_3dLayer;
    m_stretch = other.m_stretch;
    m_parentLayer = other.m_parentLayer;
    m_td = other.m_td;
    m_clipMode = other.m_clipMode;
    if (other.m_effects) {
        m_effects = new QLottieBase;
        for (QLottieBase *effect : other.m_effects->children())
            m_effects->appendChild(effect->clone());
    }
    //m_transformAtFirstFrame = other.m_transformAtFirstFrame;
}

QLottieLayer::~QLottieLayer()
{
    if (m_effects)
        delete m_effects;
}

QLottieBase *QLottieLayer::clone() const
{
    return new QLottieLayer(*this);
}

QLottieLayer *QLottieLayer::construct(QJsonObject definition, const QVersionNumber &version)
{
    qCDebug(lcLottieQtLottieParser) << "QLottieLayer::construct()";

    QLottieLayer *layer = nullptr;
    int type = definition.value(QLatin1String("ty")).toInt();
    switch (type) {
    case 2:
        qCDebug(lcLottieQtLottieParser) << "Parse image layer";
        layer = new QLottieImageLayer(definition, version);
        break;
    case 4:
        qCDebug(lcLottieQtLottieParser) << "Parse shape layer";
        layer = new QLottieShapeLayer(definition, version);
        break;
    default:
        qCWarning(lcLottieQtLottieParser) << "Unsupported layer type:" << type;
    }
    return layer;
}

bool QLottieLayer::active(int frame) const
{
    return (!m_hidden && (frame >= m_startFrame && frame <= m_endFrame));
}

void QLottieLayer::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return;

    qCDebug(lcLottieQtLottieParser) << "QLottieLayer::parse():" << m_name;

    m_layerIndex = definition.value(QLatin1String("ind")).toVariant().toInt();
    m_startFrame = definition.value(QLatin1String("ip")).toVariant().toInt();
    m_endFrame = definition.value(QLatin1String("op")).toVariant().toInt();
    m_startTime = definition.value(QLatin1String("st")).toVariant().toReal();
    m_blendMode = definition.value(QLatin1String("lottie")).toVariant().toInt();
    m_autoOrient = definition.value(QLatin1String("ao")).toBool();
    m_3dLayer = definition.value(QLatin1String("ddd")).toBool();
    m_stretch = definition.value(QLatin1String("sr")).toVariant().toReal();
    m_parentLayer = definition.value(QLatin1String("parent")).toVariant().toInt();
    m_td = definition.value(QLatin1String("td")).toInt();
    int clipMode = definition.value(QLatin1String("tt")).toInt(-1);
    if (clipMode > -1 && clipMode < 5)
        m_clipMode = static_cast<MatteClipMode>(clipMode);

    QJsonArray effects = definition.value(QLatin1String("ef")).toArray();
    parseEffects(effects);

    if (m_clipMode > 2)
        qCWarning(lcLottieQtLottieParser)
                << "Lottie Layer: Only alpha mask layer supported:" << m_clipMode;
    if (m_blendMode > 0)
        qCWarning(lcLottieQtLottieParser)
                << "Lottie Layer: Unsupported blend mode" << m_blendMode;
    if (m_stretch > 1)
        qCWarning(lcLottieQtLottieParser)
                << "Lottie Layer: stretch not supported" << m_stretch;
    if (m_autoOrient)
        qCWarning(lcLottieQtLottieParser)
                << "Lottie Layer: auto-orient not supported";
    if (m_3dLayer)
        qCWarning(lcLottieQtLottieParser)
                << "Lottie Layer: is a 3D layer, but not handled";
}

void QLottieLayer::updateProperties(int frame)
{
    if (m_parentLayer)
        resolveLinkedLayer();

    // Update first effects, as they are not children of the layer
    if (m_effects) {
        for (QLottieBase* effect : m_effects->children())
            effect->updateProperties(frame);
    }

    QLottieBase::updateProperties(frame);
}

void QLottieLayer::render(QLottieRenderer &renderer) const
{
    // Render first effects, as they affect the children
    renderEffects(renderer);

    QLottieBase::render(renderer);
}

QLottieBase *QLottieLayer::findChild(const QString &childName)
{
    QLottieBase *child = nullptr;

    if (m_effects)
        child = m_effects->findChild(childName);

    if (child)
        return child;
    else
        return QLottieBase::findChild(childName);
}

QLottieLayer *QLottieLayer::resolveLinkedLayer()
{
    if (m_linkedLayer)
        return m_linkedLayer;

    resolveTopRoot();

    Q_ASSERT(topRoot());

    for (QLottieBase *child : topRoot()->children()) {
        QLottieLayer *layer = static_cast<QLottieLayer*>(child);
        if (layer->layerId() == m_parentLayer) {
            m_linkedLayer = layer;
            break;
        }
    }
    return m_linkedLayer;
}

QLottieLayer *QLottieLayer::linkedLayer() const
{
    return m_linkedLayer;
}

bool QLottieLayer::isClippedLayer() const
{
    return m_clipMode != NoClip;
}

bool QLottieLayer::isMaskLayer() const
{
    return m_td > 0;
}

QLottieLayer::MatteClipMode QLottieLayer::clipMode() const
{
    return m_clipMode;
}

int QLottieLayer::layerId() const
{
    return m_layerIndex;
}

QLottieBasicTransform *QLottieLayer::transform() const
{
    return m_layerTransform;
}

void QLottieLayer::renderEffects(QLottieRenderer &renderer) const
{
    if (!m_effects)
        return;

    for (QLottieBase* effect : m_effects->children()) {
        if (effect->hidden())
            continue;
        effect->render(renderer);
    }
}

void QLottieLayer::parseEffects(const QJsonArray &definition, QLottieBase *effectRoot)
{
    QJsonArray::const_iterator it = definition.constEnd();
    while (it != definition.constBegin()) {
        // Create effects container if at least one effect found
        if (!m_effects) {
            m_effects = new QLottieBase;
            effectRoot = m_effects;
        }
        it--;
        QJsonObject effect = (*it).toObject();
        int type = effect.value(QLatin1String("ty")).toInt();
        switch (type) {
        case 0:
        {
            QLottieBase *slider = new QLottieBase;
            slider->parse(effect);
            effectRoot->appendChild(slider);
            break;
        }
        case 5:
        {
            if (effect.value(QLatin1String("en")).toInt()) {
                QLottieBase *group = new QLottieBase;
                group->parse(effect);
                effectRoot->appendChild(group);
                parseEffects(effect.value(QLatin1String("ef")).toArray(), group);
            }
            break;
        }
        case 21:
        {
            QLottieFillEffect *fill = new QLottieFillEffect;
            fill->construct(effect, m_version);
            effectRoot->appendChild(fill);
            break;
        }
        default:
            qCWarning(lcLottieQtLottieParser)
                << "QLottieLayer: Unsupported effect" << type;
        }
    }
}

QT_END_NAMESPACE
