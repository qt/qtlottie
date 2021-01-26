/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the lottie-qt module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include "lottierenderer_p.h"

QT_BEGIN_NAMESPACE

void LottieRenderer::setTrimmingState(LottieRenderer::TrimmingState trimmingState)
{
    m_trimmingState = trimmingState;
}

LottieRenderer::TrimmingState LottieRenderer::trimmingState() const
{
    return m_trimmingState;
}

void LottieRenderer::saveTrimmingState()
{
    m_trimStateStack.push(m_trimmingState);
}

void LottieRenderer::restoreTrimmingState()
{
    if (m_trimStateStack.count())
        m_trimmingState = m_trimStateStack.pop();
}

QT_END_NAMESPACE
