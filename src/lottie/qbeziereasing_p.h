// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QBEZIEREASING_P_H
#define QBEZIEREASING_P_H

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

#include <private/qbezier_p.h>

QT_BEGIN_NAMESPACE

class QBezierEasing
{
public:
    void addCubicBezierSegment(const QPointF &c1, const QPointF &c2, const QPointF &endPoint);
    qreal valueForProgress(qreal progress) const;

private:
    qreal tForX(qreal x) const;
    QBezier mBezier;
};

QT_END_NAMESPACE

#endif // QBEZIEREASING_P_H
