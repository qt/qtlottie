// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qlottievisitor_p.h"
#include <private/qquickgenerator_p.h>
#include <private/qquicknodeinfo_p.h>
#include <QtLottie/private/qlottieshape_p.h>
#include <QtLottie/private/qlottiestroke_p.h>
#include <QtLottie/private/qlottierect_p.h>
#include <QtLottie/private/qlottiepolystar_p.h>
#include <QtLottie/private/qlottieellipse_p.h>
#include <QtLottie/private/qlottiefreeformshape_p.h>
#include <QtLottie/private/qlottieshapetransform_p.h>
#include <QtLottie/private/qlottiegfill_p.h>
#include <QtLottie/private/qlottieround_p.h>
#include <QtLottie/private/qlottieroot_p.h>
#include <QtLottie/private/qlottieflatlayers_p.h>
#include <QtLottie/private/qlottieprecomplayer_p.h>
#include <QtLottie/private/qlottieimage_p.h>
#include <QtLottie/private/qlottieprecomposition_p.h>
#include <QtLottie/private/qlottietextlayer_p.h>

#include <QtGui/private/qfixed_p.h>

#include <QFile>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcLottieQtVisitor, "qt.lottieqt.visitor")

using namespace Qt::Literals::StringLiterals;

#define QLOTTIEVISITOR_DEBUG \
    qCDebug(lcLottieQtVisitor).noquote().nospace() \
        << QByteArray().fill(' ', m_savedPaintInfos.size() * 4) \
        << ((trimmingState() == Sequential) ? QByteArray("trimmed") : QByteArray{})

QLottieVisitor::QLottieVisitor(const QString lottieFileName, QQuickGenerator *generator)
    : m_lottieFileName(lottieFileName), m_generator(generator)
{
}

void QLottieVisitor::render(const QLottieRoot &root)
{
    m_currentFrameCounterIds.push(idForNode(&root));
    m_currentFrameLimits.push({ root.startFrame(), root.endFrame() });
    StructureNodeInfo info;
    fillCommonNodeInfo(&root, &info);

    m_frameRate = root.frameRate();

    info.size = root.layerSize();
    info.viewBox = QRectF(QPointF(0, 0), info.size);

    QLOTTIEVISITOR_DEBUG << "[root viewbox=" << info.viewBox << ", frame rate=" << m_frameRate << "]";

    info.stage = StructureNodeStage::Start;
    info.nodeId = "_q_animation"_L1; // # centralize

    TimelineInfo tlInfo;
    tlInfo.startFrame = root.startFrame();
    tlInfo.endFrame = root.endFrame();
    tlInfo.frameRate = m_frameRate;
    info.timelineInfo = tlInfo;

    m_generator->generateRootNode(info);

    m_precompositions = root.precompositions();
    generatePrecompositions();

    root.render(*this);

    info.stage = StructureNodeStage::End;
    m_generator->generateRootNode(info);
    m_currentFrameLimits.pop();
    m_currentFrameCounterIds.pop();
}

void QLottieVisitor::render(const QLottiePrecomposition &precomp)
{
    // Instantiate (inside a PrecompLayer) a precomp that has been defined earlier

    StructureNodeInfo info;
    fillCommonNodeInfo(&precomp, &info);
    info.nodeId = scrub(precomp.refId());
    info.defsId = idForNode(m_precompositions.value(precomp.refId()));
    if (m_currentBoundsIds.isEmpty())
        info.bounds = QRect(QPoint(), precomp.layerSize());
    else
        info.boundsReferenceId = m_currentBoundsIds.top();
    TimelineInfo tlInfo;
    tlInfo.frameCounterReference = m_currentFrameCounterIds.top();
    tlInfo.generateFrameCounter = true;
    info.timelineInfo = tlInfo;
    m_generator->generateDefsInstantiationNode(info);

    info.stage = StructureNodeStage::End;
    m_generator->generateDefsInstantiationNode(info);
}

QString QLottieVisitor::idForNode(const QLottieBase *node)
{
    QString idForNull;
    QString &id = node ? m_idForNodeId[node] : idForNull;
    if (id.isNull())
        id = QStringLiteral("_qt_node%1").arg(m_nodeIdCounter++);
    return id;
}

QString QLottieVisitor::scrub(const QString &raw)
{
    QString res(raw.left(80));

    if (!res.isEmpty()) {
        constexpr QLatin1StringView legalSymbols("_-.:/[](){}*| "); // No quot. mark or backslash!
        qsizetype i = 0;
        do {
            if (res.at(i).isLetterOrNumber() || legalSymbols.contains(res.at(i)))
                i++;
            else
                res.remove(i, 1);
        } while (i < res.size());
    }

    return res;
}

void QLottieVisitor::fillCommonNodeInfo(const QLottieBase *node,
                                        NodeInfo *info,
                                        const QString &suffix)
{
    info->id = idForNode(node) + suffix;
    if (node != nullptr) {
        info->nodeId = scrub(node->name());
        info->typeName = QStringLiteral("Type%1").arg(node->type());
    }
    info->transform.setTimelineReferenceId(m_currentFrameCounterIds.top());
    info->opacity.setTimelineReferenceId(m_currentFrameCounterIds.top());
    info->motionPath.setTimelineReferenceId(m_currentFrameCounterIds.top());
}

void QLottieVisitor::fillAnimationNodeInfo(const QLottieBase *node, NodeInfo *info)
{
    Q_UNUSED(node);
    for (const PaintInfo::TransformAnimationInfo &animInfo : std::as_const(m_currentPaintInfo.transformAnimations)) {
        if (!animInfo.frames.isEmpty()) {
            const bool hasPaths = !animInfo.motionPath.isEmpty();

            QQuickAnimatedProperty::PropertyAnimation animation;
            animation.frames = animInfo.frames;
            animation.easingPerFrame = animInfo.easingPerFrame;
            polishPropertyAnimation(&animation);

            if (animInfo.animationType == QTransform::TxTranslate && hasPaths) {
                // Default value holds additional parameters
                QVariantList params({ QVariant::fromValue(animInfo.motionPath),
                                      QVariant(animInfo.isAutoOrienting) });
                info->motionPath.setDefaultValue(params);
                info->motionPath.addAnimation(animation);
            } else {
                animation.subtype = animInfo.animationType;
                if (animInfo.animationType == QTransform::TxNone)
                    info->opacity.addAnimation(animation);
                else
                    info->transform.addAnimation(animation);
            }
        }
    }
}

void QLottieVisitor::saveState()
{
    m_savedPaintInfos.append(m_currentPaintInfo);
}

void QLottieVisitor::restoreState()
{
    Q_ASSERT(!m_savedPaintInfos.isEmpty());
    m_currentPaintInfo = m_savedPaintInfos.takeLast();
}

void QLottieVisitor::generateMatteNode(const QLottieLayer *layer, StructureNodeStage stage)
{
    const QString suffix(QStringLiteral("_box"));

    if (layer->isMatteLayer()) {
        StructureNodeInfo info;
        fillCommonNodeInfo(layer, &info, suffix);
        if (m_currentBoundsIds.isEmpty())
            info.bounds = QRect(QPoint(), layer->layerSize());
        else
            info.boundsReferenceId = m_currentBoundsIds.top();
        info.transformReferenceChildId = idForNode(layer);
        if (stage == StructureNodeStage::Start) {
            info.stage = StructureNodeStage::Start;
            if (!m_generator->generateDefsNode(info))
                return;
        }

        MaskNodeInfo maskInfo;
        maskInfo.stage = stage;
        fillCommonNodeInfo(layer, &maskInfo, suffix);
        maskInfo.nodeId.clear();
        maskInfo.typeName.clear();
        if (m_currentBoundsIds.isEmpty())
            maskInfo.bounds = QRect(QPoint(), layer->layerSize());
        else
            maskInfo.boundsReferenceId = m_currentBoundsIds.top();
        maskInfo.maskRect = maskInfo.bounds;
        m_generator->generateMaskNode(maskInfo);

        if (stage != StructureNodeStage::Start) {
            info.stage = StructureNodeStage::End;
            if (!m_generator->generateDefsNode(info))
                return;
        }

    } else if (layer->isUsingMatteLayer() && layer->parent()) {
        StructureNodeInfo info;
        info.stage = stage;
        fillCommonNodeInfo(layer, &info, suffix);
        info.nodeId.clear();
        info.typeName.clear();
        if (m_currentBoundsIds.isEmpty())
            info.bounds = QRect(QPoint(), layer->layerSize());
        else
            info.boundsReferenceId = m_currentBoundsIds.top();
        const QLottieLayer::MatteClipMode mode = layer->matteMode();
        info.isMaskAlpha = (mode == QLottieLayer::Alpha || mode == QLottieLayer::InvertedAlpha);
        info.isMaskInverted = (mode == QLottieLayer::InvertedAlpha || mode == QLottieLayer::InvertedLuminence);
        // Find id of matte layer, assume preceding layer
        const QList<QLottieBase *> &siblings = layer->parent()->children();
        const QLottieBase *prevSibling = siblings.value(siblings.indexOf(layer) - 1);
        const QLottieLayer *precedingLayer = QLottieLayer::checkedCast(prevSibling);
        if (precedingLayer && precedingLayer->isMatteLayer()) {
            info.maskId = idForNode(precedingLayer) + suffix;
            m_generator->generateStructureNode(info);
        }
        QLOTTIEVISITOR_DEBUG << "  matte link resolved to layer: " << info.maskId;
    }
}

void QLottieVisitor::render(const QLottieLayer &layer)
{
    QLOTTIEVISITOR_DEBUG << "[layer '" << layer.name() << "' type " << Qt::hex << layer.type()
                         << (layer.isMatteLayer() ? " matte" : "") << "]";

    m_currentFrameLimits.push({ layer.startFrame(), layer.endFrame() });

    if (layer.isMatteLayer() || layer.isUsingMatteLayer())
        generateMatteNode(&layer, StructureNodeStage::Start);

    StructureNodeInfo info;
    fillCommonNodeInfo(&layer, &info);
    info.customItemType = QStringLiteral("LayerItem");
    info.stage = StructureNodeStage::Start;
    info.transform.setDefaultValue(QVariant::fromValue(m_currentPaintInfo.transform));
    info.isDefaultTransform = m_currentPaintInfo.transform.isIdentity();
    info.opacity.setDefaultValue(m_currentPaintInfo.opacity);
    info.isDefaultOpacity = qFuzzyCompare(m_currentPaintInfo.opacity, 1.0);
    if (layer.type() == LOTTIE_LAYER_PRECOMP_IX || layer.type() == LOTTIE_LAYER_SOLID_IX) {
        info.bounds = QRectF({}, layer.layerSize());
        m_currentBoundsIds.push(info.id);
    }

    bool setFrameCounterReference = false;
    TimelineInfo tlInfo;
    tlInfo.startFrame = layer.startFrame();
    tlInfo.endFrame = layer.endFrame();
    tlInfo.generateVisibility = true;
    tlInfo.frameCounterReference = m_currentFrameCounterIds.top();
    if (layer.type() == LOTTIE_LAYER_PRECOMP_IX) {
        const QLottiePrecompLayer &pcLayer = static_cast<const QLottiePrecompLayer &>(layer);
        registerScalarAnimation(&tlInfo.frameCounterMapper, pcLayer.timeRemapProperty(), m_frameRate);
        qreal offset = pcLayer.frameOffset();
        bool hasMultiplier = pcLayer.timeStretch() != 0 && pcLayer.timeStretch() != 1;
        if (tlInfo.frameCounterMapper.isAnimated() || offset || hasMultiplier) {
            tlInfo.generateFrameCounter = true;
            setFrameCounterReference = true;
            if (offset)
                tlInfo.frameCounterOffset = -offset;
            if (hasMultiplier)
                tlInfo.frameCounterMultiplier = qreal(1) / pcLayer.timeStretch();
        }
    }
    info.timelineInfo = tlInfo;

    fillAnimationNodeInfo(&layer, &info);
    if (layer.hasLinkedLayer() && layer.parent()) {
        for (const QLottieBase *sibling : layer.parent()->children()) {
            if (auto siblingLayer = QLottieLayer::checkedCast(sibling)) {
                if (siblingLayer != &layer && siblingLayer->layerId() == layer.linkedLayerId()) {
                    info.transformReferenceId = idForNode(siblingLayer);
                    if (siblingLayer->isMatteLayer())
                        info.transformReferenceId += QStringLiteral("_box.item");
                }
            }
        }
        QLOTTIEVISITOR_DEBUG << "  xf link resolved to layer: " << info.transformReferenceId;
    }

    m_generator->generateStructureNode(info);

    m_currentPaintInfo = {};
    if (setFrameCounterReference)
        m_currentFrameCounterIds.push(info.id);
}

void QLottieVisitor::render(const QLottieSolidLayer &layer)
{
    render(static_cast<const QLottieLayer &>(layer));
    if (!layer.layerSize().isEmpty()) {
        m_currentPaintInfo.fill = layer.color();
        QPainterPath layerRect;
        layerRect.addRect(QRect(QPoint(), layer.layerSize()));
        setPathNodeStaticPath(layerRect);
        generatePathNode(nullptr);
    }
}

void QLottieVisitor::render(const QLottieTextLayer &layer)
{
    QLOTTIEVISITOR_DEBUG << "[text layer '" << layer.name() << "']";

    render(static_cast<const QLottieLayer &>(layer));

    const QList<QPainterPath> glyphPaths = layer.buildGlyphPaths();
    if (glyphPaths.isEmpty())
        return;

    m_currentPaintInfo.fill = layer.textDocument().fillColor;
    m_currentPaintInfo.fillRule = Qt::WindingFill;

    StructureNodeInfo info;
    fillCommonNodeInfo(&layer, &info, "_glyphs"_L1);
    info.stage = StructureNodeStage::Start;
    info.isPathContainer = true;

    m_generator->generateStructureNode(info);
    m_currentStructElements.push(&layer);

    for (const QPainterPath &path : glyphPaths) {
        setPathNodeStaticPath(path);
        generatePathNode(nullptr);
    }

    info.stage = StructureNodeStage::End;
    m_generator->generateStructureNode(info);
    m_currentStructElements.pop();
}

void QLottieVisitor::render(const QLottieGroup &group)
{
    QLOTTIEVISITOR_DEBUG << "[group '" << group.name() << "' #children " << group.children().size() << "]";
    if (group.children().isEmpty())
        return;

    bool hasPaths = false;
    bool hasGroups = false;
    for (const QLottieBase *child : group.children()) {
        if (child->type() == LOTTIE_SHAPE_GROUP_IX)
            hasGroups = true;
        else if (child->isPathElement())
            hasPaths = true;
    }

    // There are two cases where we want to generate a structure node for a LottieGroup
    // 1) If the group contains concrete shapes, then we want a Shape node to contain the paths
    // 2) If the group contains (sub)groups and has a transform. Since the subgroups will have
    //    their own transforms, this group's transform must be applied first with a structure node

    const QLottieBase *groupXf = group.children().first();
    if (groupXf->type() == LOTTIE_SHAPE_TRANS_IX) { // Always true in wellformed lottie file
        // We must apply the group xf already here, so it will become the structure node's xf.
        // If non-identity, m_currentStructElements is used to avoid re-applying it on normal visit.
        render(*static_cast<const QLottieShapeTransform *>(groupXf));
    }
    const bool groupHasTransform = !m_currentPaintInfo.transform.isIdentity()
            || m_currentPaintInfo.transformAnimations.size() > 0;
    if (hasPaths || (hasGroups && groupHasTransform)) {
        StructureNodeInfo info;
        fillCommonNodeInfo(&group, &info);
        info.stage = StructureNodeStage::Start;
        info.isPathContainer = hasPaths;
        info.transform.setDefaultValue(QVariant::fromValue(m_currentPaintInfo.transform));
        info.isDefaultTransform = m_currentPaintInfo.transform.isIdentity();
        info.opacity.setDefaultValue(m_currentPaintInfo.opacity);
        info.isDefaultOpacity = qFuzzyCompare(m_currentPaintInfo.opacity, 1.0);

        fillAnimationNodeInfo(&group, &info);

        m_generator->generateStructureNode(info);
        m_currentStructElements.push(&group);

        m_currentPaintInfo.transform.reset();
        m_currentPaintInfo.transformAnimations.clear();
        m_currentPaintInfo.opacity = 1.0;
    }
}

void QLottieVisitor::finish(const QLottieLayer &layer)
{
    QLOTTIEVISITOR_DEBUG << "[layer '" << layer.name() << "' finish]";

    if (layer.type() == LOTTIE_LAYER_PRECOMP_IX) {
        if (m_currentFrameCounterIds.top() == idForNode(&layer))
            m_currentFrameCounterIds.pop();
    }

    StructureNodeInfo info;
    info.stage = StructureNodeStage::End;
    fillCommonNodeInfo(&layer, &info);
    m_generator->generateStructureNode(info);

    if (!m_currentBoundsIds.isEmpty() && m_currentBoundsIds.top() == idForNode(&layer))
        m_currentBoundsIds.pop();

    if (layer.isMatteLayer() || layer.isUsingMatteLayer())
        generateMatteNode(&layer, StructureNodeStage::End);

    m_currentFrameLimits.pop();
}

void QLottieVisitor::finish(const QLottieGroup &group)
{
    QLOTTIEVISITOR_DEBUG << "[group '" << group.name() << "' finish]";

    if (!m_currentStructElements.isEmpty() && m_currentStructElements.top() == &group) {
        bool hasPaths = false;
        for (const QLottieBase *child : group.children()) {
            if (child->isPathElement()) {
                hasPaths = true;
                break;
            }
        }

        StructureNodeInfo info;
        info.stage = StructureNodeStage::End;
        info.isPathContainer = hasPaths;

        fillCommonNodeInfo(&group, &info);
        m_generator->generateStructureNode(info);
        m_currentStructElements.pop();
    }
}

// Actual shape rendering handled by renderPathElements() instead:

void QLottieVisitor::render(const QLottieRect &rect)
{
    QLOTTIEVISITOR_DEBUG << "[rect]";
    Q_UNUSED(rect);
}

void QLottieVisitor::render(const QLottieEllipse &ellipse)
{
    QLOTTIEVISITOR_DEBUG << "[ellipse]";
    Q_UNUSED(ellipse);
}

void QLottieVisitor::render(const QLottiePolyStar &star)
{
    QLOTTIEVISITOR_DEBUG << "[star]";
    Q_UNUSED(star);
}

void QLottieVisitor::render(const QLottieFreeFormShape &shape)
{
    QLOTTIEVISITOR_DEBUG << "[freeform]";
    Q_UNUSED(shape);
}

void QLottieVisitor::render(const QLottieRound &round)
{
    QLOTTIEVISITOR_DEBUG << "[round]";

    // ### Not implemented: path rounding modifier
    Q_UNUSED(round);
}

void QLottieVisitor::render(const QLottieFill &fill)
{
    QLOTTIEVISITOR_DEBUG << "[fill color=" << fill.color() << ", opacity=" << fill.opacity() << "]";

    QColor color = fill.color();
    color.setAlphaF(color.alphaF() * (fill.opacity() / 100.0));
    m_currentPaintInfo.fill = color;
    m_currentPaintInfo.fillColorAnimation = makeColorAnimation(fill.colorProperty());
    m_currentPaintInfo.fillOpacityAnimation = makeScalarAnimation(fill.opacityProperty(), qreal(1) / 100);
    m_currentPaintInfo.fillRule = fill.fillRule();
    m_currentPaintInfo.inverseFillTransform.reset();
}

void QLottieVisitor::render(const QLottieGFill &gradient)
{
    QLOTTIEVISITOR_DEBUG << "[fill gradient]";

    if (gradient.value() != nullptr)
        m_currentPaintInfo.fill = *gradient.value();
    m_currentPaintInfo.fillRule = gradient.fillRule();
    m_currentPaintInfo.inverseFillTransform.reset();
}

void QLottieVisitor::render(const QLottieImage &image)
{
    QLOTTIEVISITOR_DEBUG << "[image size=" << image.size() << "]";

    ImageNodeInfo info;
    fillCommonNodeInfo(&image, &info);
    info.image = image.image();
    info.rect = QRectF(QPointF(), image.size());
    info.externalFileReference = image.url().toLocalFile();

    m_generator->generateImageNode(info);
}

void QLottieVisitor::render(const QLottieStroke &stroke)
{
    QLOTTIEVISITOR_DEBUG << "[stroke color=" << stroke.pen().color()
                         << ", opacity=" << stroke.opacity() << "]";

    const QPen pen = stroke.pen();
    m_currentPaintInfo.stroke = pen;

    if (m_currentPaintInfo.stroke.brush().gradient() == nullptr) {
        QColor color = pen.color();
        color.setAlphaF(color.alphaF() * (stroke.opacity() / 100.0));
        m_currentPaintInfo.stroke.setColor(color);
    }

    m_currentPaintInfo.strokeColorAnimation = makeColorAnimation(stroke.colorProperty());
    m_currentPaintInfo.strokeOpacityAnimation = makeScalarAnimation(stroke.opacityProperty(), qreal(1) / 100);
    m_currentPaintInfo.strokeWidthAnimation = makeScalarAnimation(stroke.widthProperty());
    m_currentPaintInfo.strokeDashOffsetAnimation = makeScalarAnimation(stroke.dashOffsetProperty(), qreal(1) / pen.widthF());
}

void QLottieVisitor::render(const QLottieBasicTransform &transform)
{
    QLOTTIEVISITOR_DEBUG << "[basic transform s=" << transform.scale()
                         << ", r=" << transform.rotation()
                         << ", o=" << transform.opacity() << "]";

    const bool isShapeTransform = false;
    bool autoOrient = false;
    if (const QLottieLayer *layer = QLottieLayer::checkedCast(transform.parent()))
        autoOrient = layer->isAutoOrienting();

    if (hasAnimations(&transform))
        collectTransformAnimations(&transform, isShapeTransform, autoOrient);
    else
        applyTransform(&m_currentPaintInfo.transform, transform, false);

    m_currentPaintInfo.opacity *= transform.opacity();
}

namespace {
    template<typename T>
    QLottieVisitor::PaintInfo::TransformAnimationInfo
        collectAnimations(const T &property,
                          QTransform::TransformationType type,
                          std::function<void(qreal,
                                             const QVariant &,
                                             QLottieVisitor::PaintInfo::TransformAnimationInfo *,
                                             std::optional<QBezier>)> storeAnimationFrame,
                          std::function<QVariantList(const QVariant &)> createParams)
    {
        const auto easingCurves = property.easingCurves();
        QLottieVisitor::PaintInfo::TransformAnimationInfo info;
        info.animationType = type;
        if (easingCurves.isEmpty()) {
            const QVariantList params = createParams(QVariant::fromValue(property.value()));
            storeAnimationFrame(0, params, &info, std::nullopt);
        } else {
            std::optional<QBezier> easingBezier;
            for (const auto &curve : easingCurves) {
                const auto value = curve.startValue;
                const QVariantList params = createParams(QVariant::fromValue(value));
                storeAnimationFrame(curve.startFrame, params, &info, easingBezier);

                easingBezier = curve.easing.bezier(); // belongs to generator's next keyframe
            }
        }
        return info;
    }
}

void QLottieVisitor::collectTransformAnimations(const QLottieBasicTransform *transform,
                                                bool isShapeTransform, bool autoOrient)
{
    Q_UNUSED(isShapeTransform);
    const QLottieProperty<QPointF> anchorPoints = transform->anchorPointProperty();
    const QLottieProperty<qreal> rotations = transform->rotationProperty();
    const QLottieProperty<QPointF> scales = transform->scaleProperty();
    const QLottieSpatialProperty positions = transform->positionProperty();
    const QLottieProperty<qreal> opacities = transform->opacityProperty();
    const QLottieProperty<qreal> xPositions = transform->xPosProperty();
    const QLottieProperty<qreal> yPositions = transform->yPosProperty();

    const bool positionHasCurves = positions.hasCurves();

    auto storeAnimationFrame = [&](qreal lottieFrameNumber,
                                   const QVariant &propertyValue,
                                   PaintInfo::TransformAnimationInfo *info,
                                   std::optional<QBezier> easingBezier = std::nullopt) {
        const int timePoint = qRound(lottieFrameNumber);
        info->frames[timePoint] = propertyValue;
        if (easingBezier)
            info->easingPerFrame[timePoint] = *easingBezier;
    };

    QLottieVisitor::PaintInfo::TransformAnimationInfo info;
    if (!transform->splitPosition()) {
        if (positionHasCurves || autoOrient) { // Use the motionPath property
            const auto easingCurves = positions.easingCurves();
            const auto &pathSegments = positions.subPaths();
            Q_ASSERT(!pathSegments.isEmpty());

            // Combine motion segments to one path; calculate segment lengths for progress steps
            QPainterPath combinedPath;
            QList<qreal> segmentLengths;
            segmentLengths.reserve(pathSegments.size());
            qreal totalLength = 0;
            for (const QPainterPath &pathSegment : pathSegments) {
                segmentLengths.append(pathSegment.length());
                totalLength += segmentLengths.last();
                combinedPath.connectPath(pathSegment);
            }

            QLottieVisitor::PaintInfo::TransformAnimationInfo info;
            info.animationType = QTransform::TxTranslate;
            info.motionPath = combinedPath;
            info.isAutoOrienting = autoOrient;

            qreal accumLength = 0;
            std::optional<QBezier> easingBezier;
            for (qsizetype i = 0; i < easingCurves.size(); ++i) {
                const auto &curve = easingCurves.at(i);
                qreal tValue = accumLength / totalLength;
                storeAnimationFrame(curve.startFrame, tValue, &info, easingBezier);

                accumLength += segmentLengths[i];
                easingBezier = curve.easing.bezier(); // belongs to generator's next keyframe
            }

            m_currentPaintInfo.transformAnimations.append(info);
        } else {
            info = collectAnimations(positions,
                                     QTransform::TxTranslate,
                                     storeAnimationFrame,
                                     [](const QVariant &v)
                                     {
                                         return QVariantList{ v };
                                     });
        }
        m_currentPaintInfo.transformAnimations.append(info);
    } else {
        info = collectAnimations(xPositions,
                                 QTransform::TxTranslate,
                                 storeAnimationFrame,
                                 [](const QVariant &v)
                                 {
                                     return QVariantList{ QVariant::fromValue(QPointF(v.toReal(), 0.0)) };
                                 });
        m_currentPaintInfo.transformAnimations.append(info);

        info = collectAnimations(yPositions,
                                 QTransform::TxTranslate,
                                 storeAnimationFrame,
                                 [](const QVariant &v)
                                 {
                                     return QVariantList{ QVariant::fromValue(QPointF(0.0, v.toReal())) };
                                 });
        m_currentPaintInfo.transformAnimations.append(info);
    }

    auto storeRotationParameter = [](const QVariant &v) {
        return QVariantList{ QVariant::fromValue(QPointF(0, 0)), v };
    };
    info = collectAnimations(rotations,
                             QTransform::TxRotate,
                             storeAnimationFrame,
                             storeRotationParameter);
    m_currentPaintInfo.transformAnimations.append(info);

    if (isShapeTransform) {
        const QLottieShapeTransform *shapeTransform =
            static_cast<const QLottieShapeTransform *>(transform);

        const QLottieProperty<qreal> skews = shapeTransform->skewProperty();
        const QLottieProperty<qreal> skewAxes = shapeTransform->skewAxisProperty();

        // Lottie shear transforms work by first rotating by skew axis angle, then applying
        // the skewAngle as the shear along the X-axis, and then rotating back.
        info = collectAnimations(skewAxes,
                                 QTransform::TxRotate,
                                 storeAnimationFrame,
                                 [](const QVariant &v) {
                                     return QVariantList{ QVariant::fromValue(QPointF(0, 0)),
                                                          QVariant::fromValue(-1.0 * v.toReal()) };
                                 });
        m_currentPaintInfo.transformAnimations.append(info);

        info = collectAnimations(skews,
                                 QTransform::TxShear,
                                 storeAnimationFrame,
                                 [](const QVariant &v) {
                                     return QVariantList{ QVariant::fromValue(QPointF(-1.0 * v.toReal(), 0.0)) };
                                 });
        m_currentPaintInfo.transformAnimations.append(info);

        info = collectAnimations(skewAxes,
                                 QTransform::TxRotate,
                                 storeAnimationFrame,
                                 storeRotationParameter);
        m_currentPaintInfo.transformAnimations.append(info);
    }

    info = collectAnimations(scales,
                             QTransform::TxScale,
                             storeAnimationFrame,
                             [](const QVariant &v) {
                                 return QVariantList{ QVariant::fromValue(v.toPointF() / 100.0) };
                             });
    m_currentPaintInfo.transformAnimations.append(info);

    info = collectAnimations(anchorPoints,
                             QTransform::TxTranslate,
                             storeAnimationFrame,
                             [](const QVariant &v) {
                                 return QVariantList{ QVariant::fromValue(v.toPointF() * -1.0) };
                             });
    m_currentPaintInfo.transformAnimations.append(info);

    {
        const QList<EasingSegment<qreal> > easingCurves = opacities.easingCurves();
        PaintInfo::TransformAnimationInfo info;
        info.animationType = QTransform::TxNone;

        std::optional<QBezier> easingBezier;
        for (const auto &curve : easingCurves) {
            const auto value = curve.startValue / 100;
            const QVariant params = QVariant::fromValue(value);
            storeAnimationFrame(curve.startFrame, params, &info, easingBezier);

            easingBezier = curve.easing.bezier(); // For next keyframe
        }
        m_currentPaintInfo.transformAnimations.append(info);
    }
}

void QLottieVisitor::registerScalarAnimation(QQuickAnimatedProperty *outProperty,
                                             const QLottieProperty<qreal> &inProperty,
                                             qreal scale)
{
    if (outProperty->animationCount() > 0)
        return; // Animation already registered on earlier visit
    QQuickAnimatedProperty::PropertyAnimation animation = makeScalarAnimation(inProperty, scale);
    if (!animation.frames.isEmpty()) {
        outProperty->addAnimation(animation);
        outProperty->setTimelineReferenceId(m_currentFrameCounterIds.top());
    }
}

QQuickAnimatedProperty::PropertyAnimation QLottieVisitor::makeColorAnimation(const QLottieProperty4D<QVector4D> &colorProperty)
{
    QQuickAnimatedProperty::PropertyAnimation animation;

    std::optional<QBezier> easingBezier;
    for (const auto &curve : colorProperty.easingCurves()) {
        const QVector4D raw = curve.startValue;
        QColor value = QColor::fromRgbF(raw.x(), raw.y(), raw.z(), raw.w());
        int frameTime = qRound(curve.startFrame);
        animation.frames[frameTime] = value;
        if (easingBezier)
            animation.easingPerFrame[frameTime] = *easingBezier;
        easingBezier = curve.easing.bezier(); // For next keyframe
    }
    polishPropertyAnimation(&animation);
    return animation;
}

QQuickAnimatedProperty::PropertyAnimation QLottieVisitor::makeScalarAnimation(
        const QLottieProperty<qreal> &scalarProperty, qreal scale)
{
    QQuickAnimatedProperty::PropertyAnimation animation;

    std::optional<QBezier> easingBezier;
    for (const auto &curve : scalarProperty.easingCurves()) {
        qreal value = curve.startValue * scale;
        int frameTime = qRound(curve.startFrame);
        animation.frames[frameTime] = value;
        if (easingBezier)
            animation.easingPerFrame[frameTime] = *easingBezier;
        easingBezier = curve.easing.bezier(); // For next keyframe
    }
    polishPropertyAnimation(&animation);
    return animation;
}

void QLottieVisitor::polishPropertyAnimation(QQuickAnimatedProperty::PropertyAnimation *animation)
{
    animation->flags |= QQuickAnimatedProperty::PropertyAnimation::FreezeAtEnd;
    // Add boundary keyframe(s) as necessary to ensure correct behavior of generated code
    if (animation && !animation->frames.isEmpty()) {
        const int layerStartFrame = m_currentFrameLimits.top().first;
        if (animation->frames.firstKey() > layerStartFrame)
            animation->frames[layerStartFrame] = animation->frames.first();
    }
}

bool QLottieVisitor::hasAnimations(const QLottieBasicTransform *transform, bool isShapeTransform)
{
    bool hasAnimations =
            transform->rotationProperty().startFrame() < transform->rotationProperty().endFrame()
            || transform->positionProperty().startFrame() < transform->positionProperty().endFrame()
            || transform->scaleProperty().startFrame() < transform->scaleProperty().endFrame()
            || transform->opacityProperty().startFrame() < transform->opacityProperty().endFrame()
            || transform->anchorPointProperty().startFrame()
                    < transform->anchorPointProperty().endFrame();

    if (transform->splitPosition() && !hasAnimations) {
        hasAnimations = transform->xPosProperty().startFrame() < transform->xPosProperty().endFrame()
                        || transform->yPosProperty().startFrame() < transform->yPosProperty().endFrame();
    }

    if (isShapeTransform && !hasAnimations) {
        const QLottieShapeTransform *shapeTransform = static_cast<const QLottieShapeTransform *>(transform);
        hasAnimations = shapeTransform->skewProperty().startFrame() < shapeTransform->skewProperty().endFrame()
            || shapeTransform->skewAxisProperty().startFrame() < shapeTransform->skewAxisProperty().endFrame();
    }

    if (qEnvironmentVariableIntValue("QLOTTIEVISITOR_DISABLE_ANIMATIONS"))
        return false;

    return hasAnimations;
}

void QLottieVisitor::render(const QLottieShapeTransform &transform)
{
    if (!m_currentStructElements.isEmpty() && transform.parent() == m_currentStructElements.top()) {
        // This transform was already applied as part of a group structure node
        return;
    }

    QLOTTIEVISITOR_DEBUG << "[shape transform p=" << transform.position()
                         << ", s=" << transform.scale()
                         << ", r=" << transform.rotation()
                         << ", o=" << transform.opacity() << "]";

    if (hasAnimations(&transform, true))
        collectTransformAnimations(&transform, true);
    else
        applyTransform(&m_currentPaintInfo.transform, transform, true);

    // A gradient fill defined in enclosing group scope must have its coords mapped back
    if (m_currentPaintInfo.fill.gradient())
        applyTransform(&m_currentPaintInfo.inverseFillTransform, transform, true);

    m_currentPaintInfo.opacity *= transform.opacity();
}

void QLottieVisitor::render(const QLottieTrimPath &trim)
{
    QLOTTIEVISITOR_DEBUG << "[trim, isParallel: " << trim.isParallel() << "]";

    m_currentPaintInfo.trim.enabled = true;
    m_currentPaintInfo.trim.start.setDefaultValue(trim.start() / 100.0);
    m_currentPaintInfo.trim.end.setDefaultValue(trim.end() / 100.0);
    m_currentPaintInfo.trim.offset.setDefaultValue(trim.offset() / 360.0);

    registerScalarAnimation(&m_currentPaintInfo.trim.start, trim.startProperty(), 1.0 / 100.0);
    registerScalarAnimation(&m_currentPaintInfo.trim.end, trim.endProperty(), 1.0 / 100.0);
    registerScalarAnimation(&m_currentPaintInfo.trim.offset, trim.offsetProperty(), 1.0 / 360.0);

    if (!trim.isParallel())
        generatePathNode(&trim);
}

void QLottieVisitor::render(const QLottieFillEffect &effect)
{
    QLOTTIEVISITOR_DEBUG << "[effect]";

    // ### What are you?
    Q_UNUSED(effect);
}

void QLottieVisitor::render(const QLottieRepeater &repeater)
{
    QLOTTIEVISITOR_DEBUG << "[repeater]";

    // ### Repeats the following shapes N times with different transforms
    Q_UNUSED(repeater);
}

void QLottieVisitor::fillPathAnimationInfo(const QList<QLottieBase *> &pathElements)
{
    QList<int> keyFrameNumbers;
    QList<int> firstAnimNumbers;
    QList<QBezier> firstAnimEasings;

    auto registerAnimation = [&](const auto &property) {
        if (!property.isAnimated())
            return;
        const bool isFirstAnim = firstAnimNumbers.isEmpty();
        for (const auto &curve : property.easingCurves()) {
            const int timePoint = qRound(curve.startFrame);
            keyFrameNumbers.append(timePoint);
            if (isFirstAnim) {
                firstAnimNumbers.append(timePoint);
                firstAnimEasings.append(curve.easing.bezier());
            }
        }
    };

    QList<QLottieShape *> elementCopies;
    elementCopies.reserve(pathElements.size());
    for (const QLottieBase *element : pathElements) {
        switch (element->type()) {
        case LOTTIE_SHAPE_SHAPE_IX: {
            const auto *ffShape = static_cast<const QLottieFreeFormShape *>(element);
            if (ffShape->isAnimated()) {
                registerAnimation(ffShape->startPointProperty());
            }
            elementCopies.append(new QLottieFreeFormShape(*ffShape));
            break;
        }
        case LOTTIE_SHAPE_RECT_IX: {
            const auto *rect = static_cast<const QLottieRect *>(element);
            if (rect->isAnimated()) {
                registerAnimation(rect->sizeProperty());
                registerAnimation(rect->positionProperty());
            }
            elementCopies.append(new QLottieRect(*rect));
            break;
        }
        case LOTTIE_SHAPE_ELLIPSE_IX: {
            const auto *ellipse = static_cast<const QLottieEllipse *>(element);
            if (ellipse->isAnimated()) {
                registerAnimation(ellipse->sizeProperty());
                registerAnimation(ellipse->positionProperty());
            }
            elementCopies.append(new QLottieEllipse(*ellipse));
            break;
        }
        case LOTTIE_SHAPE_STAR_IX: {
            const auto *star = static_cast<const QLottiePolyStar *>(element);
            if (star->isAnimated()) {
                registerAnimation(star->outerRadiusProperty());
                registerAnimation(star->innerRadiusProperty());
                registerAnimation(star->positionProperty());
            }
            elementCopies.append(new QLottiePolyStar(*star));
            break;
        }
        default:
            continue;
        }
    }

    std::sort(keyFrameNumbers.begin(), keyFrameNumbers.end());
    keyFrameNumbers.erase(std::unique(keyFrameNumbers.begin(), keyFrameNumbers.end()),
                          keyFrameNumbers.end());

    QQuickAnimatedProperty::PropertyAnimation pa;
    for (int frame : std::as_const(keyFrameNumbers)) {
        QPainterPath path;
        for (QLottieShape *shape : elementCopies) {
            if (shape->isAnimated())
                shape->updateProperties(frame);
            QPainterPath elementPath = shape->path();
            if (elementPath.isEmpty())
                elementPath = shape->fallbackPath();
            path.addPath(elementPath);
        }
        pa.frames[frame] = QVariant::fromValue(path);
    }
    qDeleteAll(elementCopies);
    elementCopies.clear();

    if (keyFrameNumbers.size() == firstAnimNumbers.size()) {
        // I.e. identical timepoints, so we can use the first anim's easings to the combined path
        // (This is fully correct only if there is only one animation, or if all have same easings)
        for (int i = 1; i < keyFrameNumbers.size(); i++)
            pa.easingPerFrame[keyFrameNumbers[i]] = firstAnimEasings.value(i - 1);
    }

    polishPropertyAnimation(&pa);
    m_currentPaintInfo.path.addAnimation(pa);
}

void QLottieVisitor::renderPathElements_helper(const QList<QLottieBase *> &pathElements)
{
    QPainterPath defaultPath;
    const QLottieShape *firstShape = nullptr;

    bool hasAnimatedElements = false;
    for (const QLottieBase *element : pathElements) {
        if (!element->isPathElement())
            continue;
        const QLottieShape *shape = static_cast<const QLottieShape *>(element);
        if (!firstShape)
            firstShape = shape;
        defaultPath.addPath(shape->path());
        hasAnimatedElements = hasAnimatedElements || shape->isAnimated();
    }
    m_currentPaintInfo.path.setDefaultValue(QVariant::fromValue(defaultPath));

    if (hasAnimatedElements)
        fillPathAnimationInfo(pathElements);

    if (trimmingState() != Sequential)
        generatePathNode(firstShape);
}

void QLottieVisitor::renderPathElements(const QList<QLottieBase *> &pathElements)
{
    QLOTTIEVISITOR_DEBUG << "[path elements, count = " << pathElements.size() << "]";

    const bool needsIndividualPathNodes = (trimmingState() == Parallel);
    if (!needsIndividualPathNodes) {
        renderPathElements_helper(pathElements);
    } else {
        for (int i = 0; i < pathElements.size(); i++)
            renderPathElements_helper(pathElements.sliced(i, 1));
    }
}

void QLottieVisitor::generatePrecompositions()
{
    // We need a relative, not absolute, reference to the instantiating item, since a precomp can
    // be instantiated multiple times, possibly with different bounds and frameCounter offsets
    QString instantiatiorRef = QStringLiteral("parent"); // ref to Loader, indirectly PrecompLayer

    for (const auto &[key, precomp] : m_precompositions.asKeyValueRange()) {
        // Generate the wrapping defs node start
        StructureNodeInfo info;
        info.stage = StructureNodeStage::Start;
        fillCommonNodeInfo(precomp, &info);
        info.nodeId = scrub(key);
        info.boundsReferenceId = instantiatiorRef;
        TimelineInfo tlInfo;
        tlInfo.frameCounterReference = instantiatiorRef;
        tlInfo.generateFrameCounter = true;
        info.timelineInfo = tlInfo;

        m_generator->generateDefsNode(info);

        // Generate the precomp's layers
        const QString precompId = info.id + QStringLiteral("_defs"); // defs node item
        m_currentBoundsIds.push(precompId);
        m_currentFrameCounterIds.push(precompId);
        precomp->renderChildren(*this);
        m_currentFrameCounterIds.pop();
        m_currentBoundsIds.pop();

        // End defs wrapper
        info.stage = StructureNodeStage::End;
        m_generator->generateDefsNode(info);
    }
}

void QLottieVisitor::generatePathNode(const QLottieShape *shape)
{
    QLOTTIEVISITOR_DEBUG << "[drawing shape with"
                         << " stroke=" << m_currentPaintInfo.stroke
                         << ", fill=" << m_currentPaintInfo.fill;

    if (m_currentPaintInfo.path.defaultValue().value<QPainterPath>().isEmpty()
        && !m_currentPaintInfo.path.isAnimated())
        return;

    StructureNodeInfo info;
    if (m_currentStructElements.isEmpty()) {
        fillCommonNodeInfo(shape, &info);
        info.stage = StructureNodeStage::Start;
        info.isPathContainer = true;

        info.transform.setDefaultValue(QVariant::fromValue(m_currentPaintInfo.transform));
        info.isDefaultTransform = m_currentPaintInfo.transform.isIdentity();
        info.opacity.setDefaultValue(m_currentPaintInfo.opacity);
        info.isDefaultOpacity = qFuzzyCompare(m_currentPaintInfo.opacity, 1.0);

        fillAnimationNodeInfo(shape, &info);

        m_generator->generateStructureNode(info);
    }

    PathNodeInfo pathInfo;
    fillCommonNodeInfo(shape, &pathInfo, QStringLiteral("_path"));

    pathInfo.path = m_currentPaintInfo.path;
    pathInfo.fillRule = m_currentPaintInfo.fillRule;
    pathInfo.fillColor.setDefaultValue(QVariant::fromValue(m_currentPaintInfo.fill.color()));
    if (!m_currentPaintInfo.fillColorAnimation.isConstant())
        pathInfo.fillColor.addAnimation(m_currentPaintInfo.fillColorAnimation);
    if (!m_currentPaintInfo.fillOpacityAnimation.isConstant())
        pathInfo.fillOpacity.addAnimation(m_currentPaintInfo.fillOpacityAnimation);

    if (m_currentPaintInfo.stroke.style() != Qt::NoPen) {
        pathInfo.strokeStyle = StrokeStyle::fromPen(m_currentPaintInfo.stroke);
        pathInfo.strokeStyle.cosmetic = false;
        pathInfo.strokeStyle.color.setDefaultValue(QVariant::fromValue(m_currentPaintInfo.stroke.color()));
        if (!m_currentPaintInfo.strokeColorAnimation.isConstant())
            pathInfo.strokeStyle.color.addAnimation(m_currentPaintInfo.strokeColorAnimation);
        if (!m_currentPaintInfo.strokeOpacityAnimation.isConstant())
            pathInfo.strokeStyle.opacity.addAnimation(m_currentPaintInfo.strokeOpacityAnimation);
        if (!m_currentPaintInfo.strokeWidthAnimation.isConstant())
            pathInfo.strokeStyle.width.addAnimation(m_currentPaintInfo.strokeWidthAnimation);
        if (!m_currentPaintInfo.strokeDashOffsetAnimation.isConstant())
            pathInfo.strokeStyle.dashOffset.addAnimation(m_currentPaintInfo.strokeDashOffsetAnimation);
        if (m_currentPaintInfo.stroke.brush().gradient() != nullptr)
            pathInfo.strokeGrad = *m_currentPaintInfo.stroke.brush().gradient();
    }

    if (m_currentPaintInfo.fill.gradient() != nullptr)
        pathInfo.grad = *m_currentPaintInfo.fill.gradient();
    if (!m_currentPaintInfo.inverseFillTransform.isIdentity())
        pathInfo.fillTransform = m_currentPaintInfo.inverseFillTransform.inverted();
    if (trimmingState() != TrimmingState::Off)
        pathInfo.trim = m_currentPaintInfo.trim;

    pathInfo.path.setTimelineReferenceId(m_currentFrameCounterIds.top());
    pathInfo.fillColor.setTimelineReferenceId(m_currentFrameCounterIds.top());
    pathInfo.fillOpacity.setTimelineReferenceId(m_currentFrameCounterIds.top());
    pathInfo.strokeStyle.color.setTimelineReferenceId(m_currentFrameCounterIds.top());
    pathInfo.strokeStyle.opacity.setTimelineReferenceId(m_currentFrameCounterIds.top());
    pathInfo.strokeStyle.width.setTimelineReferenceId(m_currentFrameCounterIds.top());
    pathInfo.strokeStyle.dashOffset.setTimelineReferenceId(m_currentFrameCounterIds.top());
    m_generator->generatePath(pathInfo);

    if (m_currentStructElements.isEmpty()) {
        info.stage = StructureNodeStage::End;
        m_generator->generateStructureNode(info);
    }

    m_currentPaintInfo.path = QQuickAnimatedProperty(QVariant::fromValue(QPainterPath{ }));
}

void QLottieVisitor::setPathNodeStaticPath(const QPainterPath &path)
{
    QLOTTIEVISITOR_DEBUG << "[path bounds=" << path.controlPointRect() << "]";

    m_currentPaintInfo.path.setDefaultValue(QVariant::fromValue(path));
}

QT_END_NAMESPACE
