// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qlottieanimationbuilder_p.h"

#include <private/qquicktimeline_p.h>
#include <private/qquicklayeritem_p.h>
#include <private/qquickanimationrootitem_p.h>

#include <QtGui/private/qbezier_p.h>
#include <QtQml/qqmlparserstatus.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcLottieQtAnimationBuilder, "qt.lottieqt.animationbuilder")

static void completeParserStatus(QObject *obj)
{
    if (auto *ps = qobject_cast<QQmlParserStatus *>(obj))
        ps->componentComplete();
}

static int processAnimationTime(int timeMs)
{
    static const qreal multiplier =
            qEnvironmentVariable("QT_QUICKVECTORIMAGE_TIME_DILATION", QStringLiteral("1.0"))
                    .toDouble();
    return qRound(multiplier * timeMs);
}

static QEasingCurve easingForFrame(const QQuickAnimatedProperty::PropertyAnimation &anim, int frame,
                                   QMap<std::array<qreal, 4>, QEasingCurve> &cache)
{
    QEasingCurve easing;
    auto it = anim.easingPerFrame.constFind(frame);
    if (it == anim.easingPerFrame.constEnd())
        return easing;

    const QBezier &bezier = it.value();
    const QPointF c1 = bezier.pt2();
    const QPointF c2 = bezier.pt3();
    if (c1 == c1.transposed() && c2 == c2.transposed())
        return easing; // linear

    const std::array<qreal, 4> key{ c1.x(), c1.y(), c2.x(), c2.y() };
    auto cacheIt = cache.constFind(key);
    if (cacheIt != cache.constEnd())
        return cacheIt.value();

    easing.setType(QEasingCurve::BezierSpline);
    easing.addCubicBezierSegment(c1, c2, QPointF(1, 1));
    cache.insert(key, easing);
    return easing;
}

QQuickAbstractAnimation *LottieAnimationBuilder::enterTimelineScope(QQuickItem *item,
                                                                    const StructureNodeInfo &info)
{
    Q_ASSERT(info.timelineInfo);
    const TimelineInfo &timelineInfo = *info.timelineInfo;
    QQuickTimeline *timeline = nullptr;
    QQuickTimelineAnimation *masterAnimation = nullptr;
    bool ownsTimeline = true;
    auto *root = qobject_cast<QQuickAnimationRootItem *>(item);
    QQuickTimeline *referencedTimeline = nullptr;

    if (!m_scopes.isEmpty()) {
        referencedTimeline = timelineForReference(timelineInfo.frameCounterReference);
        if (!referencedTimeline) {
            qCWarning(lcLottieQtAnimationBuilder) << "enterTimelineScope: no timeline for reference"
                                                  << timelineInfo.frameCounterReference
                                                  << "- falling back to the enclosing scope";
            referencedTimeline = m_scopes.top().timeline;
        }
    }

    if (m_scopes.isEmpty()) {
        timeline = new QQuickTimeline(item);
        timeline->setStartFrame(timelineInfo.startFrame);
        timeline->setEndFrame(timelineInfo.endFrame);

        masterAnimation = new QQuickTimelineAnimation(item);
        masterAnimation->setObjectName(u"_qt_frameCounterAnimation"_s);
        masterAnimation->setTargetObject(timeline);
        masterAnimation->setProperty(u"currentFrame"_s);
        masterAnimation->setFrom(timelineInfo.startFrame);
        masterAnimation->setTo(timelineInfo.endFrame);
        const int durationMs = int(1000.0 * qAbs(timelineInfo.endFrame - timelineInfo.startFrame)
                                   / qMax(timelineInfo.frameRate, 1));
        masterAnimation->setDuration(processAnimationTime(durationMs));
        auto animsProp = timeline->animations();
        animsProp.append(&animsProp, masterAnimation);

        if (root)
            root->setFrameCounter(timelineInfo.startFrame);
        else
            item->setProperty("frameCounter", timelineInfo.startFrame);
        QObject::connect(timeline, &QQuickTimeline::currentFrameChanged, item,
                         [item, timeline, root]() {
                             if (root)
                                 root->setFrameCounter(timeline->currentFrame());
                             else
                                 item->setProperty("frameCounter", timeline->currentFrame());
                         });
    } else if (!timelineInfo.generateFrameCounter) {
        timeline = referencedTimeline;
        ownsTimeline = false;
    } else {
        QQuickTimeline *parentTimeline = referencedTimeline;
        timeline = new QQuickTimeline(item);
        timeline->setStartFrame(timelineInfo.startFrame);
        timeline->setEndFrame(timelineInfo.endFrame);
        if (timelineInfo.frameCounterMapper.isAnimated()) {
            addKeyframeGroup(parentTimeline, timeline, "currentFrame",
                             timelineInfo.frameCounterMapper.animation(0));
        } else {
            const qreal offset = timelineInfo.frameCounterOffset;
            const qreal multiplier = timelineInfo.frameCounterMultiplier
                    ? timelineInfo.frameCounterMultiplier
                    : qreal(1);
            timeline->setCurrentFrame((parentTimeline->currentFrame() + offset) * multiplier);
            QObject::connect(parentTimeline, &QQuickTimeline::currentFrameChanged, timeline,
                             [timeline, parentTimeline, offset, multiplier]() {
                                 timeline->setCurrentFrame(
                                         (parentTimeline->currentFrame() + offset) * multiplier);
                             });
        }
    }

    if (timelineInfo.generateVisibility) {
        auto *visibilityTimeline = referencedTimeline ? referencedTimeline : timeline;

        const qreal startF = timelineInfo.startFrame;
        const qreal endF = timelineInfo.endFrame;
        item->setVisible(visibilityTimeline->currentFrame() >= startF
                         && visibilityTimeline->currentFrame() < endF);
        QObject::connect(visibilityTimeline, &QQuickTimeline::currentFrameChanged, item,
                         [item, visibilityTimeline, startF, endF]() {
                             const qreal v = visibilityTimeline->currentFrame();
                             item->setVisible(v >= startF && v < endF);
                         });
    }

    m_scopes.push({ timeline, masterAnimation, ownsTimeline, info.id });
    return masterAnimation;
}

void LottieAnimationBuilder::enterTimelineScope(const QString &scopeId, const QString &referenceId)
{
    if (m_scopes.isEmpty())
        return;

    QQuickTimeline *timeline = timelineForReference(referenceId);
    if (!timeline) {
        qCWarning(lcLottieQtAnimationBuilder)
                << "enterTimelineScope: no timeline for reference" << referenceId
                << "- falling back to the enclosing scope for" << scopeId;
        timeline = m_scopes.top().timeline;
    }

    m_scopes.push({ timeline, nullptr, false, scopeId });
}

void LottieAnimationBuilder::exitTimelineScope()
{
    if (m_scopes.isEmpty())
        return;

    const Scope scope = m_scopes.pop();
    if (!scope.ownsTimeline)
        return;

    completeParserStatus(scope.timeline);
    scope.timeline->setEnabled(true);

    if (scope.masterAnimation) {
        completeParserStatus(scope.masterAnimation);
        scope.masterAnimation->setRunning(true);
    }
}

void LottieAnimationBuilder::bindProperty(QObject *target, const QByteArray &propertyName,
                                          const QQuickAnimatedProperty &property)
{
    if (m_scopes.isEmpty() || !target || property.animationCount() == 0)
        return;

    const QString referenceId = property.timelineReferenceId();
    QQuickTimeline *timeline = timelineForReference(referenceId);
    if (!timeline) {
        qCWarning(lcLottieQtAnimationBuilder)
                << "bindProperty: no timeline for reference" << referenceId
                << "- falling back to the enclosing scope for" << target << propertyName;
        timeline = m_scopes.top().timeline;
    }

    addKeyframeGroup(timeline, target, propertyName, property.animation(0));
}

QQuickTimeline *LottieAnimationBuilder::timelineForReference(const QString &referenceId) const
{
    if (referenceId.isEmpty())
        return nullptr;

    for (auto it = m_scopes.crbegin(); it != m_scopes.crend(); ++it) {
        if (it->referenceId == referenceId)
            return it->timeline;
    }
    return nullptr;
}

void LottieAnimationBuilder::addKeyframeGroup(QQuickTimeline *timeline, QObject *target,
                                              const QByteArray &propertyName,
                                              const QQuickAnimatedProperty::PropertyAnimation &anim)
{
    if (anim.frames.isEmpty())
        return;

    if (anim.repeatCount != 1 || anim.startOffset
        || anim.flags != QQuickAnimatedProperty::PropertyAnimation::FreezeAtEnd) {
        qCWarning(lcLottieQtAnimationBuilder)
                << "addKeyframeGroup: animation feature not implemented in timeline mode, for"
                << target << propertyName;
    }

    auto *group = new QQuickKeyframeGroup(timeline);
    group->setTargetObject(target);
    group->setProperty(QString::fromUtf8(propertyName));

    auto keyframesProp = group->keyframes();
    for (auto it = anim.frames.constBegin(); it != anim.frames.constEnd(); ++it) {
        auto *keyframe = new QQuickKeyframe(group);
        keyframe->setFrame(it.key());
        keyframe->setValue(it.value());
        keyframe->setEasing(easingForFrame(anim, it.key(), m_easingCache));
        keyframesProp.append(&keyframesProp, keyframe);
    }
    completeParserStatus(group);

    auto groupsProp = timeline->keyframeGroups();
    groupsProp.append(&groupsProp, group);
}

QQuickItem *LottieAnimationBuilder::createCustomItem(const QString &type)
{
    if (type == "LayerItem"_L1)
        return new QQuickLayerItem();
    return nullptr;
}

QT_END_NAMESPACE
