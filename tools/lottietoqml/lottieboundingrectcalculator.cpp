// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "lottieboundingrectcalculator_p.h"

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

QT_BEGIN_NAMESPACE

LottieBoundingRectCalculator::LottieBoundingRectCalculator() {}

void LottieBoundingRectCalculator::saveState()
{
    m_transformStack.append(m_currentTransform);
    m_unitedPathStack.append(m_unitedPath);
}

void LottieBoundingRectCalculator::restoreState()
{
    m_currentTransform = m_transformStack.takeLast();
    m_unitedPath = m_unitedPathStack.takeLast();
}

void LottieBoundingRectCalculator::render(const QLottieLayer &layer)
{
    Q_UNUSED(layer);
}

void LottieBoundingRectCalculator::render(const QLottieSolidLayer &layer)
{
    Q_UNUSED(layer);
}

void LottieBoundingRectCalculator::finish(const QLottieLayer &layer)
{
    Q_UNUSED(layer);
}

void LottieBoundingRectCalculator::render(const QLottieRect &rect)
{
    processShape(rect);
}

void LottieBoundingRectCalculator::render(const QLottieEllipse &ellipse)
{
    processShape(ellipse);
}

void LottieBoundingRectCalculator::render(const QLottiePolyStar &star)
{
    processShape(star);
}

void LottieBoundingRectCalculator::render(const QLottieRound &round)
{
    processShape(round);
}

void LottieBoundingRectCalculator::render(const QLottieFill &fill)
{
    Q_UNUSED(fill);
}

void LottieBoundingRectCalculator::render(const QLottieGFill &shape)
{
    Q_UNUSED(shape);
}

void LottieBoundingRectCalculator::render(const QLottieImage &image)
{
    Q_UNUSED(image);
}

void LottieBoundingRectCalculator::render(const QLottieStroke &stroke)
{
    Q_UNUSED(stroke);
}

void LottieBoundingRectCalculator::render(const QLottieBasicTransform &transform)
{
    applyTransform(&m_currentTransform, transform);
}

void LottieBoundingRectCalculator::render(const QLottieShapeTransform &transform)
{
    applyTransform(&m_currentTransform, transform, true);
}

void LottieBoundingRectCalculator::render(const QLottieFreeFormShape &shape)
{
    processShape(shape);
}

void LottieBoundingRectCalculator::render(const QLottieTrimPath &trans)
{
    if (!trans.isParallel() && !qFuzzyIsNull(m_unitedPath.length())) {
        QPainterPath p = trans.trim(m_unitedPath);
        processShape(p);
    }
}

void LottieBoundingRectCalculator::render(const QLottieFillEffect &effect)
{
    Q_UNUSED(effect);
}

void LottieBoundingRectCalculator::render(const QLottieRepeater &repeater)
{
    Q_UNUSED(repeater);
}

void LottieBoundingRectCalculator::processShape(const QLottieShape &shape)
{
    QPainterPath p = m_currentTransform.map(shape.path());
    if (trimmingState() == Sequential) {
        p.addPath(m_unitedPath);
        m_unitedPath = p;
    } else {
        processShape(p);
    }
}

void LottieBoundingRectCalculator::processShape(const QPainterPath &path)
{
    m_currentBoundingRect = m_currentBoundingRect.united(path.boundingRect());
}

QT_END_NAMESPACE
