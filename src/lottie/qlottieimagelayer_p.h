// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEIMAGELAYER_P_H
#define QLOTTIEIMAGELAYER_P_H

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

#include <QtLottie/private/qlottielayer_p.h>

QT_BEGIN_NAMESPACE

class QJsonObject;

class QLottieRenderer;
class QLottieShape;
class QLottieTrimPath;
class QLottieBasicTransform;

class LOTTIE_EXPORT QLottieImageLayer : public QLottieLayer
{
public:
    QLottieImageLayer() = default;
    explicit QLottieImageLayer(const QLottieImageLayer &other);
    QLottieImageLayer(const QJsonObject &definition, const QVersionNumber &version);

    QLottieBase *clone() const override;

    void updateProperties(int frame) override;
    void render(QLottieRenderer &render) const override;
};

QT_END_NAMESPACE

#endif // QLOTTIEIMAGELAYER_P_H
