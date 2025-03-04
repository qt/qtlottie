// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottieshapelayer_p.h"

#include <QJsonObject>
#include <QJsonArray>


#include "qlottieconstants_p.h"
#include "qlottiebase_p.h"
#include "qlottieshape_p.h"
#include "qlottietrimpath_p.h"
#include "qlottiebasictransform_p.h"
#include "qlottierenderer_p.h"

QT_BEGIN_NAMESPACE

QLottieShapeLayer::QLottieShapeLayer(const QLottieShapeLayer &other)
    : QLottieLayer(other)
{
    m_maskProperties = other.m_maskProperties;
    m_layerTransform = new QLottieBasicTransform(*other.m_layerTransform);
    m_layerTransform->setParent(this);
    m_appliedTrim = other.m_appliedTrim;
}

QLottieShapeLayer::QLottieShapeLayer(const QJsonObject &definition, const QVersionNumber &version)
{
    m_type = LOTTIE_LAYER_SHAPE_IX;
    m_version = version;

    QLottieLayer::parse(definition);
    if (m_hidden)
        return;

    qCDebug(lcLottieQtLottieParser) << "QLottieShapeLayer::QLottieShapeLayer()"
                                       << m_name;

    QJsonArray maskProps = definition.value(QLatin1String("maskProperties")).toArray();
    QJsonArray::const_iterator propIt = maskProps.constBegin();
    while (propIt != maskProps.constEnd()) {
        m_maskProperties.append((*propIt).toVariant().toInt());
        ++propIt;
    }

    QJsonObject trans = definition.value(QLatin1String("ks")).toObject();
    m_layerTransform = new QLottieBasicTransform(trans, version, this);

    QJsonArray items = definition.value(QLatin1String("shapes")).toArray();
    QJsonArray::const_iterator itemIt = items.constEnd();
    while (itemIt != items.constBegin()) {
        itemIt--;
        QLottieShape *shape = QLottieShape::construct((*itemIt).toObject(), version, this);
        if (shape)
            appendChild(shape);
    }

    if (m_maskProperties.size())
        qCWarning(lcLottieQtLottieParser)
            << "Lottie Shape Layer: mask properties found, but not supported"
            << m_maskProperties;
}

QLottieShapeLayer::~QLottieShapeLayer()
{
    if (m_layerTransform)
        delete m_layerTransform;
}

QLottieBase *QLottieShapeLayer::clone() const
{
    return new QLottieShapeLayer(*this);
}

void QLottieShapeLayer::updateProperties(int frame)
{
    QLottieLayer::updateProperties(frame);

    m_layerTransform->updateProperties(frame);

    for (QLottieBase *child : children()) {
        if (child->hidden())
            continue;

        QLottieShape *shape = dynamic_cast<QLottieShape*>(child);

        if (!shape)
            continue;

        if (shape->type() == LOTTIE_SHAPE_TRIM_IX) {
            QLottieTrimPath *trim = static_cast<QLottieTrimPath*>(shape);
            if (m_appliedTrim)
                m_appliedTrim->applyTrim(*trim);
            else
                m_appliedTrim = trim;
        } else if (m_appliedTrim) {
            if (shape->acceptsTrim())
                shape->applyTrim(*m_appliedTrim);
        }
    }
}

void QLottieShapeLayer::render(QLottieRenderer &renderer) const
{
    renderer.saveState();

    renderEffects(renderer);

    // In case there is a linked layer, apply its transform first
    // as it affects tranforms of this layer too
    if (QLottieLayer *ll = linkedLayer())
        renderer.render(*ll->transform());

    renderer.render(*this);

    m_layerTransform->render(renderer);

    for (QLottieBase *child : children()) {
        if (child->hidden())
            continue;
        child->render(renderer);
    }

    if (m_appliedTrim && !m_appliedTrim->hidden())
        m_appliedTrim->render(renderer);

    renderer.restoreState();
}

QT_END_NAMESPACE
