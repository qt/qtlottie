// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QLOTTIEANIMATIONBUILDER_P_H
#define QLOTTIEANIMATIONBUILDER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qquickgeneratoranimationprovider_p.h>

#include <QtCore/qeasingcurve.h>
#include <QtCore/QMap>
#include <QtCore/QStack>

#include <array>

#include <QtLottieVectorImageGenerator/qtlottievectorimagegeneratorexports.h>

QT_BEGIN_NAMESPACE

class QQuickTimeline;
class QQuickTimelineAnimation;

class Q_LOTTIEVECTORIMAGEGENERATOR_EXPORT LottieAnimationBuilder : public QQuickGeneratorAnimationProvider
{
public:
    QQuickAbstractAnimation *enterTimelineScope(QQuickItem *item,
                                                const StructureNodeInfo &info) override;
    void enterTimelineScope(const QString &scopeId, const QString &referenceId) override;
    void exitTimelineScope() override;
    void bindProperty(QObject *target, const QByteArray &propertyName,
                      const QQuickAnimatedProperty &property) override;
    QQuickItem *createCustomItem(const QString &type) override;

private:
    QQuickTimeline *timelineForReference(const QString &referenceId) const;
    void addKeyframeGroup(QQuickTimeline *timeline, QObject *target, const QByteArray &propertyName,
                          const QQuickAnimatedProperty::PropertyAnimation &anim);

    struct Scope
    {
        QQuickTimeline *timeline = nullptr;
        QQuickTimelineAnimation *masterAnimation = nullptr;
        bool ownsTimeline = true;
        QString referenceId;
    };

    QStack<Scope> m_scopes;
    QMap<std::array<qreal, 4>, QEasingCurve> m_easingCache;
};

QT_END_NAMESPACE

#endif // QLOTTIEANIMATIONBUILDER_P_H
