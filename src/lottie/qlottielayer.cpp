// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qlottielayer_p.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QLoggingCategory>
#include <QtCore/QScopedValueRollback>
#include <QString>

#include "qlottieflatlayers_p.h"
#include "qlottietextlayer_p.h"
#include "qlottieshapelayer_p.h"
#include "qlottieprecomplayer_p.h"
#include "qlottiefilleffect_p.h"
#include "qlottiebasictransform_p.h"
#include "qlottierenderer_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottieLayer::QLottieLayer(const QLottieLayer &other)
    : QLottieBase(other)
{
    m_layerIndex = other.m_layerIndex;
    m_startFrame = other.m_startFrame;
    m_endFrame = other.m_endFrame;
    m_startTime = other.m_startTime;
    m_blendMode = other.m_blendMode;
    m_autoOrient = other.m_autoOrient;
    m_3dLayer = other.m_3dLayer;
    m_stretch = other.m_stretch;
    m_hasLinkedLayer = other.m_hasLinkedLayer;
    m_linkedLayerId = other.m_linkedLayerId;
    m_isMatte = other.m_isMatte;
    m_matteMode = other.m_matteMode;
    if (other.m_layerTransform) {
        m_layerTransform = new QLottieBasicTransform(*other.m_layerTransform);
        m_layerTransform->setParent(this);
    }
    m_size = other.m_size;
    if (other.m_effects) {
        m_effects = new QLottieBase;
        for (QLottieBase *effect : other.m_effects->children())
            m_effects->appendChild(effect->clone());
    }
    //m_transformAtFirstFrame = other.m_transformAtFirstFrame;
}

QLottieLayer::~QLottieLayer()
{
    if (m_layerTransform)
        delete m_layerTransform;
    if (m_effects)
        delete m_effects;
}

QLottieBase *QLottieLayer::clone() const
{
    return new QLottieLayer(*this);
}

QLottieLayer *QLottieLayer::construct(QJsonObject definition)
{
    qCDebug(lcLottieQtLottieParser) << "QLottieLayer::parse()";

    QLottieLayer *layer = nullptr;
    int type = definition.value("ty"_L1).toInt();
    int ret = 0;
    switch (type) {
    case 0:
        qCDebug(lcLottieQtLottieParser) << "Parse precomp layer";
        layer = new QLottiePrecompLayer();
        ret = layer->parse(definition);
        break;
    case 1:
        qCDebug(lcLottieQtLottieParser) << "Parse solid layer";
        layer = new QLottieSolidLayer();
        ret = layer->parse(definition);
        break;
    case 2:
        qCDebug(lcLottieQtLottieParser) << "Parse image layer";
        layer = new QLottieImageLayer();
        ret = layer->parse(definition);
        break;
    case 3:
        qCDebug(lcLottieQtLottieParser) << "Parse null layer";
        layer = new QLottieNullLayer();
        ret = layer->parse(definition);
        break;
    case 4:
        qCDebug(lcLottieQtLottieParser) << "Parse shape layer";
        layer = new QLottieShapeLayer();
        ret = layer->parse(definition);
        break;
    case 5:
        qCDebug(lcLottieQtLottieParser) << "Parse text layer";
        layer = new QLottieTextLayer();
        ret = layer->parse(definition);
        break;
    default:
        qCInfo(lcLottieQtLottieParser) << "Unsupported layer type:" << type;
    }

    if (ret < 0)
        return nullptr;

    return layer;
}

// Create a shape layer that can be used as a matte to emulate the effect of a layer's mask
QLottieLayer *QLottieLayer::constructMaskLayer(QLottieLayer *layer)
{
    static int newLayerIndex = 0x10000;

    qCDebug(lcLottieQtLottieParser) << "Parsing mask for layer" << layer->name();

    const QJsonObject &layerDef = layer->definition();
    const QJsonArray jsonMasks = layerDef.value("masksProperties"_L1).toArray();
    if (jsonMasks.isEmpty())
        return nullptr;

    QJsonObject maskLayerDef;
    maskLayerDef.insert("ty"_L1, 4);
    maskLayerDef.insert("nm"_L1, layer->name().prepend(QStringLiteral("qt_maskLayer_for_")));
    maskLayerDef.insert("ip"_L1, layerDef.value("ip"_L1));
    maskLayerDef.insert("op"_L1, layerDef.value("op"_L1));
    maskLayerDef.insert("td"_L1, 1); // i.e.: this layer will be used as matte
    maskLayerDef.insert("ks"_L1, QJsonObject{});
    // Mirror the masked layer's transform by using Lottie's transform reference ("parent") feature
    // Requires the masked layer to have a non-0 layer index, so it can be referenced
    int maskedLayerIndex = layer->m_layerIndex ? layer->m_layerIndex : newLayerIndex++;
    maskLayerDef.insert("parent"_L1, maskedLayerIndex);

    QJsonObject color({ { "a"_L1, 0 }, { "k"_L1, QJsonArray({ 1, 1, 1 }) } });
    QJsonObject shapeXf;
    shapeXf.insert("ty"_L1, "tr"_L1);
    QJsonArray shapes;
    for (const QJsonValue &maskValue : jsonMasks) {
        QJsonObject maskDef = maskValue.toObject();
        QJsonValue shapeDef = maskDef.value("pt"_L1);
        if (shapeDef.isUndefined())
            continue;
        const auto maskMode = maskDef.value("mode"_L1).toStringView("a"_L1);
        if (maskMode == "n"_L1)
            continue;
        if (maskMode != "a"_L1)
            qCInfo(lcLottieQtLottieParser) << "Unsupported mask mode" << maskMode;
        if (maskDef.value("inv"_L1).toBool())
            qCInfo(lcLottieQtLottieParser) << "Unsupported inverted mask";

        QJsonObject shape;
        shape["ty"_L1] = QJsonValue("sh"_L1);
        shape["ks"_L1] = shapeDef;

        QJsonArray items;
        items.append(shape);
        QJsonObject fill;
        fill.insert("ty"_L1, "fl"_L1);
        fill.insert("c"_L1, color);
        fill.insert("o"_L1, maskDef.value("o"_L1));
        items.append(fill);
        items.append(shapeXf);

        QJsonObject groupDef;
        groupDef["ty"_L1] = QJsonValue("gr"_L1);
        groupDef["it"_L1] = items;

        shapes.append(groupDef);
    }
    if (shapes.isEmpty())
        return nullptr;
    maskLayerDef["shapes"_L1] = shapes;

    // For debugging: qDebug() << QJsonDocument(maskLayerDef).toJson().constData();
    QLottieLayer *res = QLottieLayer::construct(maskLayerDef);
    if (res) {
        layer->m_matteMode = MatteClipMode::Alpha;
        layer->m_layerIndex = maskedLayerIndex;
    }
    return res;
}

// Take the content of a lottie layers tag and construct the corresponding layer objects
// Also adds them as children to given parent
int QLottieLayer::constructLayers(QJsonArray jsonLayers, QLottieBase *parent,
                                  const QMap<QString, QJsonObject> &assets)
{
    if (jsonLayers.size() == 0) {
        qCWarning(lcLottieQtLottieParser) << "Layers are empty";
        return -1;
    }

    int layersAdded = 0;
    QJsonArray::const_iterator jsonLayerIt = jsonLayers.constEnd();
    while (jsonLayerIt != jsonLayers.constBegin()) {
        jsonLayerIt--;
        QJsonObject jsonLayer = (*jsonLayerIt).toObject();
        if (!jsonLayer.contains("ty"_L1)) {
            qCWarning(lcLottieQtLottieParser) << "Layer" << jsonLayer.value("nm"_L1).toString()
                                              << "is missing required key \"ty\"";
            return -1;
        }

        if (jsonLayer.value("ty"_L1).toInt() == 2) {
            if (!jsonLayer.contains("refId"_L1)) {
                qCWarning(lcLottieQtLottieParser) << "Layer" << jsonLayer.value("nm"_L1).toString()
                                                  << "is missing required key \"refId\"";
                return -1;
            }

            QString refId = jsonLayer.value("refId"_L1).toString();
            jsonLayer.insert("asset"_L1, assets.value(refId));
        }

        QLottieLayer *layer = QLottieLayer::construct(jsonLayer);
        if (layer) {
            layer->setParent(parent);
            // Matte layers must be rendered before the layer they apply to, even though they
            // appear after in the list of layers. Hence, we move matte layers in front of
            // the layer they (by default) apply to, so it will be rendered first
            if (layer->isMatteLayer())
                parent->insertChildBeforeLast(layer);
            else
                parent->appendChild(layer);
            layersAdded++;

            if (layer->type() != LOTTIE_LAYER_NULL_IX && jsonLayer.value("hasMask"_L1).toBool()) {
                if (layer->isMatteLayer() || layer->isUsingMatteLayer()) {
                    qCInfo(lcLottieQtLottieParser) << "Ignoring mask of layer" << layer->name();
                } else {
                    // Create a matte that implements the layer mask
                    QLottieLayer *maskLayer = constructMaskLayer(layer);
                    if (maskLayer) {
                        maskLayer->setParent(parent);
                        parent->insertChildBeforeLast(maskLayer);
                        layersAdded++;
                    }
                }
            }
        }
    }
    return layersAdded;
}

bool QLottieLayer::active(int frame) const
{
    return (!m_hidden && ((frame >= m_startFrame && frame <= m_endFrame) || isStructureDumping()));
}

int QLottieLayer::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottieLayer::parse():" << m_name;

    m_layerIndex = definition.value("ind"_L1).toVariant().toInt();
    if (!checkRequiredKeys(definition, "Layer"_L1, { "ip"_L1, "op"_L1, "ks"_L1 }, m_name))
        return -1;
    m_startFrame = definition.value("ip"_L1).toVariant().toInt();
    m_endFrame = definition.value("op"_L1).toVariant().toInt();
    m_blendMode = definition.value("lottie"_L1).toVariant().toInt();
    m_autoOrient = (definition.value("ao"_L1).toInt() == 1);
    m_3dLayer = definition.value("ddd"_L1).toBool();
    m_linkedLayerId = definition.value("parent"_L1).toVariant().toInt(&m_hasLinkedLayer);
    m_isMatte = definition.value("td"_L1).toInt() == 1;
    int matteMode = definition.value("tt"_L1).toInt(-1);
    if (matteMode > -1 && matteMode < 5)
        m_matteMode = static_cast<MatteClipMode>(matteMode);

    QJsonObject trans = definition.value("ks"_L1).toObject();
    m_layerTransform = new QLottieBasicTransform(this);
    if (m_layerTransform->parse(trans) < 0)
        return -1;

    QJsonArray effects = definition.value("ef"_L1).toArray();
    parseEffects(effects);

    if (m_blendMode > 0)
        qCInfo(lcLottieQtLottieParser)
                << "Lottie Layer: Unsupported blend mode" << m_blendMode;
    if (m_3dLayer)
        qCInfo(lcLottieQtLottieParser)
                << "Lottie Layer: is a 3D layer, but not handled";

    return 0;
}

void QLottieLayer::updateProperties(int frame)
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

    QLottieBase::updateProperties(frame);
}

void QLottieLayer::render(QLottieRenderer &renderer) const
{
    if (!m_isActive)
        return;

    // Render first effects, as they affect the children
    renderEffects(renderer);

    // In case there is a linked layer, apply its transform first
    // as it affects tranforms of this layer too
    applyLayerTransform(renderer);

    renderer.render(*this);

    renderChildren(renderer);
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

    Q_ASSERT(parent());

    for (QLottieBase *child : parent()->children()) {
        QLottieLayer *layer = static_cast<QLottieLayer*>(child);
        if (layer->layerId() == m_linkedLayerId) {
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

bool QLottieLayer::isUsingMatteLayer() const
{
    return m_matteMode != NoClip;
}

bool QLottieLayer::isMatteLayer() const
{
    return m_isMatte;
}

QLottieLayer::MatteClipMode QLottieLayer::matteMode() const
{
    return m_matteMode;
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

void QLottieLayer::applyLayerTransform(QLottieRenderer &renderer) const
{
    if (m_applyingLayerTransform)
        return;
    QScopedValueRollback<bool> recursionGuard(m_applyingLayerTransform, true);

    if (!isStructureDumping()) {
        if (QLottieLayer *ll = linkedLayer()) {
            ll->applyLayerTransform(renderer);
            if (m_layerTransform)
                m_layerTransform->setHasLinkedLayerTransform(true);
        }
    }
    if (m_layerTransform)
        m_layerTransform->render(renderer);
}

QSize QLottieLayer::layerSize() const
{
    QSize res = m_size;
    if (!res.isValid() && parent())
        res = parent()->layerSize();
    return res;
}

const QLottieLayer *QLottieLayer::checkedCast(const QLottieBase *node)
{
    const QLottieLayer *res = nullptr;
    if (node && node->type() >= LOTTIE_LAYER_PRECOMP_IX && node->type() <= LOTTIE_LAYER_TEXT_IX)
        res = static_cast<const QLottieLayer *>(node);
    return res;
}

void QLottieLayer::parseEffects(const QJsonArray &definition, QLottieBase *effectRoot)
{
    QJsonArray::const_iterator it = definition.constEnd();
    while (it != definition.constBegin()) {
        // Create effects container if at least one effect found
        if (!effectRoot) {
            Q_ASSERT(!m_effects);
            effectRoot = new QLottieBase;
            m_effects = effectRoot;
        }
        it--;
        QJsonObject effect = (*it).toObject();
        int type = effect.value("ty"_L1).toInt();
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
            if (effect.value("en"_L1).toInt()) {
                QLottieBase *group = new QLottieBase;
                group->parse(effect);
                effectRoot->appendChild(group);
                parseEffects(effect.value("ef"_L1).toArray(), group);
            }
            break;
        }
        case 21:
        {
            QLottieFillEffect *fill = new QLottieFillEffect;
            fill->construct(effect);
            effectRoot->appendChild(fill);
            break;
        }
        default:
            qCInfo(lcLottieQtLottieParser)
                << "QLottieLayer: Unsupported effect" << type;
        }
    }
}

QT_END_NAMESPACE
