// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "lottie_plugin.h"

#include <qqml.h>

#include <QtBodymovin/private/bmconstants_p.h>

#include "lottieanimation.h"
#include "rasterrenderer/batchrenderer.h"

QT_BEGIN_NAMESPACE

void BodymovinPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<LottieAnimation>(uri, 1, 0, "LottieAnimation");
    qmlRegisterType<BMLiteral>(uri, 1, 0, "BMPropertyType");

    BatchRenderer::deleteInstance();
}

QT_END_NAMESPACE
