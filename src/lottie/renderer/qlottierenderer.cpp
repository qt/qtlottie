// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottierenderer_p.h"

QT_BEGIN_NAMESPACE

void QLottieRenderer::setTrimmingState(QLottieRenderer::TrimmingState trimmingState)
{
    m_trimmingState = trimmingState;
}

QLottieRenderer::TrimmingState QLottieRenderer::trimmingState() const
{
    return m_trimmingState;
}

void QLottieRenderer::saveTrimmingState()
{
    m_trimStateStack.push(m_trimmingState);
}

void QLottieRenderer::restoreTrimmingState()
{
    if (m_trimStateStack.size())
        m_trimmingState = m_trimStateStack.pop();
}

QT_END_NAMESPACE
