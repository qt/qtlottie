// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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

bool QLottieVisitor::nodeIsShape(const QLottieBase &node)
{
    return (node.type() >= LOTTIE_SHAPE_ELLIPSE_IX && node.type() <= LOTTIE_SHAPE_REPEATER_IX);
}

void QLottieVisitor::render(const QLottieRoot &root)
{
    enumerateLayerChildren(&root);

    StructureNodeInfo info;
    const QJsonObject rootObj = root.definition();
    info.size = QSize(rootObj.value("w"_L1).toInt(), rootObj.value("h"_L1).toInt());

    int frameRate = rootObj.value(QLatin1String("fr")).toVariant().toInt();
    if (frameRate > 0)
        m_frameRate = frameRate;

    m_duration = qRound(1000 * rootObj.value("op"_L1).toDouble(100.0) / m_frameRate);
    info.viewBox = QRectF(QPointF(0, 0), info.size);

    QLOTTIEVISITOR_DEBUG << "[root viewbox=" << info.viewBox << ", frame rate=" << m_frameRate << ", duration=" << m_duration << "ms ]";

    info.stage = StructureNodeStage::Start;
    info.nodeId = u"_q_animation"_s; // # centralize

    m_generator->generateRootNode(info);

    root.render(*this);

    info.stage = StructureNodeStage::End;
    m_generator->generateRootNode(info);
}

void QLottieVisitor::fillCommonNodeInfo(const QLottieBase *node, NodeInfo *info)
{
    Q_ASSERT(node);
    Q_ASSERT(info);

    info->typeName = QStringLiteral("Type%1").arg(node->type());
}

void QLottieVisitor::fillLayerAnimationInfo(const QLottieLayer *node, NodeInfo *info)
{
    int endTime = qRound(1000 * node->endFrame() / qreal(m_frameRate));
    if (node->startFrame() == 0 && endTime >= m_duration)
        return;

    QQuickAnimatedProperty::PropertyAnimation animation;
    if (node->startFrame() > 0)
        animation.frames[0] = QVariant::fromValue(false);
    animation.frames[qRound(1000 * node->startFrame() / qreal(m_frameRate))] = QVariant::fromValue(true);
    if (endTime < m_duration)
        animation.frames[endTime] = QVariant::fromValue(false);
    animation.frames[m_duration] = animation.frames.last();
    animation.flags |= QQuickAnimatedProperty::PropertyAnimation::FreezeAtEnd;

    info->visibility.addAnimation(animation);
}

void QLottieVisitor::fillAnimationNodeInfo(const QLottieBase *node, NodeInfo *info)
{
    Q_UNUSED(node);
    for (const PaintInfo::TransformAnimationInfo &animInfo : std::as_const(m_currentPaintInfo.transformAnimations)) {
        QQuickAnimatedProperty::PropertyAnimation animation;
        animation.subtype = animInfo.animationType;
        animation.frames = animInfo.frames;
        animation.flags |= QQuickAnimatedProperty::PropertyAnimation::FreezeAtEnd;

        if (!animation.frames.isEmpty()) {
            animation.frames[0] = animation.frames.first();
            animation.frames[m_duration] = animation.frames.last();

            // ### Support the easing bezier here
            // animation.easing = animInfo.easing;
            // animation.isDefaultEasing = false;

            if (animInfo.animationType == QTransform::TxNone)
                info->opacity.addAnimation(animation);
            else
                info->transform.addAnimation(animation);
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

void QLottieVisitor::render(const QLottieLayer &layer)
{
    QLOTTIEVISITOR_DEBUG << "[layer " << layer.name() << "type" << Qt::hex << layer.type() << "]";

    m_frameOffset += layer.frameOffset();

    StructureNodeInfo info;
    info.stage = StructureNodeStage::Start;
    if (layer.type() == LOTTIE_LAYER_PRECOMP_IX)
        info.nodeId = layer.definition().value(QLatin1String("refId")).toString();
    else
        info.nodeId = layer.name();
    info.transform.setDefaultValue(QVariant::fromValue(m_currentPaintInfo.transform));
    info.isDefaultTransform = m_currentPaintInfo.transform.isIdentity();

    fillCommonNodeInfo(&layer, &info);
    fillAnimationNodeInfo(&layer, &info);
    fillLayerAnimationInfo(&layer, &info);
    info.layerNum = m_layers.indexOf(&layer);
    if (layer.hasLinkedLayer() && layer.parent()) {
        for (const QLottieBase *sibling : layer.parent()->children()) {
            if (auto siblingLayer = QLottieLayer::checkedCast(sibling)) {
                if (siblingLayer != &layer && siblingLayer->layerId() == layer.linkedLayerId())
                    info.transformReferenceLayerNum = m_layers.indexOf(siblingLayer);
            }
        }
        QLOTTIEVISITOR_DEBUG << "  xf link resolved to layernum" << info.transformReferenceLayerNum;
    }

    m_generator->generateStructureNode(info);

    m_currentPaintInfo = {};
}

void QLottieVisitor::render(const QLottieSolidLayer &layer)
{
    render(static_cast<const QLottieLayer &>(layer));
    if (!layer.size().isEmpty()) {
        m_currentPaintInfo.fill = layer.color();
        QPainterPath layerRect;
        layerRect.addRect(QRect(QPoint(), layer.size()));
        processShape(nullptr, layerRect);
    }
}

void QLottieVisitor::finish(const QLottieLayer &layer)
{
    m_frameOffset -= layer.frameOffset();

    StructureNodeInfo info;
    info.stage = StructureNodeStage::End;

    fillCommonNodeInfo(&layer, &info);
    m_generator->generateStructureNode(info);
}

void QLottieVisitor::render(const QLottieRect &rect)
{
    QLOTTIEVISITOR_DEBUG << "[rect]";

    processShape(&rect);
}

void QLottieVisitor::render(const QLottieEllipse &ellipse)
{
    QLOTTIEVISITOR_DEBUG << "[ellipse]";

    processShape(&ellipse);
}

void QLottieVisitor::render(const QLottiePolyStar &star)
{
    QLOTTIEVISITOR_DEBUG << "[star]";

    processShape(&star);
}

void QLottieVisitor::render(const QLottieRound &round)
{
    QLOTTIEVISITOR_DEBUG << "[round]";

    processShape(&round);
}

void QLottieVisitor::render(const QLottieFill &fill)
{
    QLOTTIEVISITOR_DEBUG << "[fill color=" << fill.color() << ", opacity=" << fill.opacity() << "]";

    QColor color = fill.color();
    color.setAlphaF(color.alphaF() * (fill.opacity() / 100.0));
    m_currentPaintInfo.fill = color;

    bool isEvenOdd = (fill.definition().value(QLatin1String("r")).toInt() == 2);
    m_currentPaintInfo.fillRule = isEvenOdd ? Qt::OddEvenFill : Qt::WindingFill;
}

void QLottieVisitor::render(const QLottieGFill &gradient)
{
    QLOTTIEVISITOR_DEBUG << "[fill gradient]";

    if (gradient.value() != nullptr)
        m_currentPaintInfo.fill = *gradient.value();
    bool isEvenOdd = (gradient.definition().value(QLatin1String("r")).toInt() == 2);
    m_currentPaintInfo.fillRule = isEvenOdd ? Qt::OddEvenFill : Qt::WindingFill;
}

void QLottieVisitor::render(const QLottieImage &image)
{
    QLOTTIEVISITOR_DEBUG << "[image]";

    // ### image
    Q_UNUSED(image);
}

void QLottieVisitor::render(const QLottieStroke &stroke)
{
    QLOTTIEVISITOR_DEBUG << "[stroke color=" << stroke.pen().color()
                         << ", opacity=" << stroke.opacity() << "]";

    const QPen pen = stroke.pen();
    m_currentPaintInfo.stroke = pen;

    QColor color = pen.color();
    color.setAlphaF(color.alphaF() * (stroke.opacity() / 100.0));
    m_currentPaintInfo.stroke.setColor(color);
}

void QLottieVisitor::render(const QLottieBasicTransform &transform)
{
    QLOTTIEVISITOR_DEBUG << "[basic transform s=" << transform.scale()
                         << ", r=" << transform.rotation()
                         << ", o=" << transform.opacity() << "]";
    if (hasAnimations(&transform))
        collectTransformAnimations(&transform);
    else
        applyTransform(&m_currentPaintInfo.transform, transform, false);

    m_currentPaintInfo.opacity *= transform.opacity();
}

void QLottieVisitor::collectTransformAnimations(const QLottieBasicTransform *transform,
                                                bool isShapeTransform)
{
    Q_UNUSED(isShapeTransform);
    // ### Split position
    const QLottieProperty<QPointF> anchorPoints = transform->anchorPointProperty();
    const QLottieProperty<qreal> rotations = transform->rotationProperty();
    const QLottieProperty<QPointF> scales = transform->scaleProperty();
    const QLottieSpatialProperty positions = transform->positionProperty(); // ### Also support split positions
    const QLottieProperty<qreal> opacities = transform->opacityProperty();

    auto storeAnimationFrame = [&](qreal lottieFrameNumber, const QVariant &propertyValue,
                                   PaintInfo::TransformAnimationInfo *info) {
        const int timePointMs = qRound(1000 * (m_frameOffset + lottieFrameNumber) / m_frameRate);
        info->frames[timePointMs] = propertyValue;
    };

    {
        const QList<EasingSegment<QPointF> > easingCurves = positions.easingCurves();
        PaintInfo::TransformAnimationInfo info;
        info.animationType = QTransform::TxTranslate;
        bool firstFrame = true;
        if (easingCurves.isEmpty()) {
            const QVariantList position = { QVariant::fromValue(positions.value()) };
            storeAnimationFrame(0, position, &info);
        } else {
            for (const auto &curve : easingCurves) {
                if (firstFrame) {
                    const QPointF startValue = curve.startValue;
                    const QVariantList startParams = { QVariant::fromValue(startValue) };
                    storeAnimationFrame(0, startParams, &info);
                    if (curve.startFrame > 0)
                        storeAnimationFrame(curve.startFrame, startParams, &info);
                    firstFrame = false;
                }

                const QPointF endValue = curve.endValue;
                const QVariantList endParams = { QVariant::fromValue(endValue ) };
                storeAnimationFrame(curve.endFrame, endParams, &info);
            }
        }
        m_currentPaintInfo.transformAnimations.append(info);
    }

    {
        const QList<EasingSegment<qreal> > easingCurves = rotations.easingCurves();
        PaintInfo::TransformAnimationInfo info;
        info.animationType = QTransform::TxRotate;
        bool firstFrame = true;
        if (easingCurves.isEmpty()) {
            const QVariantList rotation = { QVariant::fromValue(QPointF(0, 0)),
                                              QVariant::fromValue(rotations.value()) };
            storeAnimationFrame(0, rotation, &info);
        } else {
            for (const auto &curve : easingCurves) {
                if (firstFrame) {
                    const qreal startValue = curve.startValue;
                    const QVariantList startParams = { QVariant::fromValue(QPointF(0, 0)),
                                                       QVariant::fromValue(startValue) };
                    storeAnimationFrame(0, startParams, &info);
                    if (curve.startFrame > 0)
                        storeAnimationFrame(curve.startFrame, startParams, &info);
                    firstFrame = false;
                }

                const qreal endValue = curve.endValue;
                const QVariantList endParams = { QVariant::fromValue(QPointF(0, 0)),
                                                 QVariant::fromValue(endValue) };
                storeAnimationFrame(curve.endFrame, endParams, &info);
            }
        }
        m_currentPaintInfo.transformAnimations.append(info);
    }

    {
        const QList<EasingSegment<QPointF> > easingCurves = scales.easingCurves();
        PaintInfo::TransformAnimationInfo info;
        info.animationType = QTransform::TxScale;
        bool firstFrame = true;

        if (easingCurves.isEmpty()) {
            const QPointF scaleValue = scales.value() / 100.0;
            const QVariantList scale = { QVariant::fromValue(scaleValue) };
            storeAnimationFrame(0, scale, &info);
        } else {
            for (const auto &curve : easingCurves) {
                if (firstFrame) {
                    const QPointF startValue = curve.startValue / 100.0;
                    const QVariantList startParams = { QVariant::fromValue(startValue) };
                    storeAnimationFrame(0, startParams, &info);
                    if (curve.startFrame > 0)
                        storeAnimationFrame(curve.startFrame, startParams, &info);
                    firstFrame = false;
                }

                const QPointF endValue = curve.endValue / 100.0;

                const QVariantList endParams = { QVariant::fromValue(endValue) };
                storeAnimationFrame(curve.endFrame, endParams, &info);
            }
        }
        m_currentPaintInfo.transformAnimations.append(info);
    }

    // ### Skew animations not implemented

    {
        const QList<EasingSegment<QPointF> > easingCurves = anchorPoints.easingCurves();
        PaintInfo::TransformAnimationInfo info;
        info.animationType = QTransform::TxTranslate;
        bool firstFrame = true;
        if (easingCurves.isEmpty()) {
            const QVariantList anchorPoint = { QVariant::fromValue(-anchorPoints.value()) };
            storeAnimationFrame(0, anchorPoint, &info);
        } else {
            for (const auto &curve : easingCurves) {
                if (firstFrame) {
                    const QPointF startValue = -curve.startValue;
                    const QVariantList startParams = { QVariant::fromValue(startValue) };
                    storeAnimationFrame(0, startParams, &info);
                    if (curve.startFrame > 0)
                        storeAnimationFrame(curve.startFrame, startParams, &info);
                    firstFrame = false;
                }

                const QPointF endValue = -curve.endValue;
                const QVariantList endParams = { QVariant::fromValue(endValue ) };
                storeAnimationFrame(curve.endFrame, endParams, &info);
            }
        }
        m_currentPaintInfo.transformAnimations.append(info);
    }

    {
        const QList<EasingSegment<qreal> > easingCurves = opacities.easingCurves();
        PaintInfo::TransformAnimationInfo info;
        info.animationType = QTransform::TxNone;
        bool firstFrame = true;
        for (const auto &curve : easingCurves) {
            if (firstFrame) {
                const qreal startValue = curve.startValue / 100;
                const QVariant startParams = QVariant::fromValue(startValue);
                storeAnimationFrame(0, startParams, &info);
                if (curve.startFrame > 0)
                    storeAnimationFrame(curve.startFrame, startParams, &info);
                firstFrame = false;
            }

            const qreal endValue = curve.endValue / 100;
            const QVariant endParams = QVariant::fromValue(endValue);
            storeAnimationFrame(curve.endFrame, endParams, &info);
        }
        m_currentPaintInfo.transformAnimations.append(info);
    }
}

void QLottieVisitor::enumerateLayerChildren(const QLottieBase *node)
{
    if (auto layer = QLottieLayer::checkedCast(node))
        m_layers.append(layer);
    for (const QLottieBase *child : node->children())
        enumerateLayerChildren(child);
}

bool QLottieVisitor::hasAnimations(const QLottieBasicTransform *transform, bool isShapeTransform)
{
    Q_UNUSED(isShapeTransform);
    bool hasAnimations = transform->rotationProperty().startFrame() < transform->rotationProperty().endFrame()
                         || transform->positionProperty().startFrame() < transform->positionProperty().endFrame()
                         || transform->scaleProperty().startFrame() < transform->scaleProperty().endFrame()
                         || transform->opacityProperty().startFrame() < transform->opacityProperty().endFrame();

    // ### Skew animations not implemented
    if (isShapeTransform && !hasAnimations) {
        const QLottieShapeTransform *shapeTransform = static_cast<const QLottieShapeTransform *>(transform);
        hasAnimations = shapeTransform->skewProperty().startFrame() < shapeTransform->skewProperty().endFrame();
    }

    if (qEnvironmentVariableIntValue("QLOTTIEVISITOR_DISABLE_ANIMATIONS"))
        return false;

    return hasAnimations;
}

void QLottieVisitor::render(const QLottieShapeTransform &transform)
{
    QLOTTIEVISITOR_DEBUG << "[shape transform s=" << transform.scale()
        << ", r=" << transform.rotation()
        << ", o=" << transform.opacity() << "]";

    if (hasAnimations(&transform, true))
        collectTransformAnimations(&transform, true);
    else
        applyTransform(&m_currentPaintInfo.transform, transform, true);

    m_currentPaintInfo.opacity *= transform.opacity();
}

void QLottieVisitor::render(const QLottieFreeFormShape &shape)
{
    QLOTTIEVISITOR_DEBUG << "[freeform]";

    processShape(&shape);
}

void QLottieVisitor::render(const QLottieTrimPath &trim)
{
    QLOTTIEVISITOR_DEBUG << "[trim]";

    m_currentPaintInfo.trim.enabled = true;
    m_currentPaintInfo.trim.start.setDefaultValue(trim.start() / 100.0);
    m_currentPaintInfo.trim.end.setDefaultValue(trim.end() / 100.0);
    m_currentPaintInfo.trim.offset.setDefaultValue(trim.offset() / 360.0);

    auto registerAnimations = [&](QQuickAnimatedProperty *outProperty,
                                  const QLottieProperty<qreal> &inProperty,
                                  qreal scale) {
        const QList<EasingSegment<qreal> > easingCurves = inProperty.easingCurves();

        QQuickAnimatedProperty::PropertyAnimation animation;
        animation.flags |= QQuickAnimatedProperty::PropertyAnimation::FreezeAtEnd;

        animation.frames[0] = outProperty->defaultValue();
        for (const auto &curve : easingCurves) {
            const qreal startValue = curve.startValue * scale;
            const qreal endValue = curve.endValue * scale;

            int startFrameTime = QFixed::fromReal(1000 * curve.startFrame / qreal(m_frameRate)).round().toInt();
            int endFrameTime = QFixed::fromReal(1000 * curve.endFrame / qreal(m_frameRate)).round().toInt();

            animation.frames[startFrameTime] = startValue;
            animation.frames[endFrameTime] = endValue;
        }
        if (!animation.frames.isEmpty()) {
            animation.frames[m_duration] = animation.frames.last();
            outProperty->addAnimation(animation);
        }
    };

    registerAnimations(&m_currentPaintInfo.trim.start, trim.startProperty(), 1.0 / 100.0);
    registerAnimations(&m_currentPaintInfo.trim.end, trim.endProperty(), 1.0 / 100.0);
    registerAnimations(&m_currentPaintInfo.trim.offset, trim.offsetProperty(), 1.0 / 360.0);

    if (!trim.isParallel())
        processShape(&trim, m_currentPaintInfo.unitedPath);
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

void QLottieVisitor::processShape(const QLottieShape *shape, const QPainterPath &path)
{
    QLOTTIEVISITOR_DEBUG << "[drawing shape with"
                         << " stroke=" << m_currentPaintInfo.stroke
                         << ", fill=" << m_currentPaintInfo.fill;

    if (path.isEmpty())
        return;
    StructureNodeInfo info;
    info.stage = StructureNodeStage::Start;
    info.isPathContainer = true;

    info.transform.setDefaultValue(QVariant::fromValue(m_currentPaintInfo.transform));
    info.isDefaultTransform = m_currentPaintInfo.transform.isIdentity();
    info.opacity.setDefaultValue(m_currentPaintInfo.opacity);
    info.isDefaultOpacity = qFuzzyCompare(m_currentPaintInfo.opacity, 1.0);

    if (shape) {
        fillCommonNodeInfo(shape, &info);
        fillAnimationNodeInfo(shape, &info);
    }

    m_generator->generateStructureNode(info);

    PathNodeInfo pathInfo;

    pathInfo.painterPath = path;
    pathInfo.fillRule = m_currentPaintInfo.fillRule;
    pathInfo.fillColor.setDefaultValue(QVariant::fromValue(m_currentPaintInfo.fill.color()));
    pathInfo.strokeStyle = StrokeStyle::fromPen(m_currentPaintInfo.stroke);
    pathInfo.strokeStyle.color.setDefaultValue(QVariant::fromValue(m_currentPaintInfo.stroke.color()));
    if (m_currentPaintInfo.fill.gradient() != nullptr)
        pathInfo.grad = *m_currentPaintInfo.fill.gradient();
    if (trimmingState() != TrimmingState::Off)
        pathInfo.trim = m_currentPaintInfo.trim;
    m_generator->generatePath(pathInfo);

    info.stage = StructureNodeStage::End;
    m_generator->generateStructureNode(info);
}

void QLottieVisitor::processShape(const QLottieShape *shape)
{
    QLOTTIEVISITOR_DEBUG << "[shape bounds=" << shape->path().controlPointRect() << "]";

    if (trimmingState() == Sequential) {
        QPainterPath p = m_currentPaintInfo.transform.map(shape->path());
        p.addPath(m_currentPaintInfo.unitedPath);
        m_currentPaintInfo.unitedPath = p;
    // } else if (m_buildingClipRegion) {     ###
    } else {
        processShape(shape, shape->path());
    }
}

QT_END_NAMESPACE
