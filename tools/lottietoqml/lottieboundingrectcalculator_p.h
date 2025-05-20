// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef LOTTIEBOUNDINGRECTCALCULATOR_P_H
#define LOTTIEBOUNDINGRECTCALCULATOR_P_H

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

#include <QtGui/qpainterpath.h>
#include <QtLottie/private/qlottierenderer_p.h>

QT_BEGIN_NAMESPACE

class QLottieShape;

class LottieBoundingRectCalculator : public QLottieRenderer
{
public:
    LottieBoundingRectCalculator();

    QRectF boundingRect() const
    {
        return m_currentBoundingRect;
    }

    void saveState() override;
    void restoreState() override;

    void render(const QLottieLayer &layer) override;
    void render(const QLottieSolidLayer &layer) override;
    void finish(const QLottieLayer &layer) override;

    void render(const QLottieRect &rect) override;
    void render(const QLottieEllipse &ellipse) override;
    void render(const QLottiePolyStar &star) override;
    void render(const QLottieRound &round) override;
    void render(const QLottieFill &fill) override;
    void render(const QLottieGFill &shape) override;
    void render(const QLottieImage &image) override;
    void render(const QLottieStroke &stroke) override;
    void render(const QLottieBasicTransform &transform) override;
    void render(const QLottieShapeTransform &transform) override;
    void render(const QLottieFreeFormShape &shape) override;
    void render(const QLottieTrimPath &trans) override;
    void render(const QLottieFillEffect &effect) override;
    void render(const QLottieRepeater &repeater) override;

private:
    void processShape(const QLottieShape &shape);
    void processShape(const QPainterPath &path);

    QPainterPath m_unitedPath;
    QRectF m_currentBoundingRect;
    QTransform m_currentTransform;

    QList<QTransform> m_transformStack;
    QList<QPainterPath> m_unitedPathStack;
};

QT_END_NAMESPACE

#endif // LOTTIEBOUNDINGRECTCALCULATOR_P_H
