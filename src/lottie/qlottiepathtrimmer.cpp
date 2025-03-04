// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottiepathtrimmer_p.h"

#include "qlottietrimpath_p.h"
#include "qlottierenderer_p.h"

#include <QPainterPath>

QT_BEGIN_NAMESPACE

QLottiePathTrimmer::QLottiePathTrimmer(QLottieBase *root)
    : m_root(root)
{
    Q_ASSERT(m_root);
}

void QLottiePathTrimmer::addTrim(QLottieTrimPath* trim)
{
    if (!trim)
        return;

    m_trimPaths.append(trim);

    if (!m_appliedTrim)
        m_appliedTrim = trim;
    else
        qCWarning(lcLottieQtLottieParser)
            << "Lottie Shape Layer: more than one trim path found on the layer."
            << "Only one (the first encountered) is supported";
}

bool QLottiePathTrimmer::inUse() const
{
    return !m_trimPaths.isEmpty();
}

void QLottiePathTrimmer::applyTrim(QLottieShape *shape)
{
    if (!m_appliedTrim)
        return;
    shape->applyTrim(*m_appliedTrim);
}

void QLottiePathTrimmer::updateProperties(int frame)
{
    QPainterPath unifiedPath;

    if (m_appliedTrim)
        m_appliedTrim->updateProperties(frame);

//    for (QLottieBase *child : m_root->children()) {
//        // TODO: Create a better system for recognizing types
//        if (child->type() >= 1000)
//            continue;

//        QLottieShape *shape = static_cast<QLottieShape*>(child);

//        // TODO: Get a better way to inherit trimming
//        if (shape->type() == LOTTIE_SHAPE_GROUP_IX && m_appliedTrim)
//            shape->applyTrim(*m_appliedTrim);

//        shape->updateProperties(frame);

//        if (m_appliedTrim && shape->acceptsTrim())
//            shape->applyTrim(*m_appliedTrim);
//    }
}

void QLottiePathTrimmer::render(QLottieRenderer &renderer) const
{
    Q_UNUSED(renderer);
//    if (m_appliedTrim) {
//        renderer.render(*m_appliedTrim);
//    }
}

QT_END_NAMESPACE
