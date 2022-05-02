/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#ifndef BMIMAGELAYER_P_H
#define BMIMAGELAYER_P_H

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

#include <QtBodymovin/private/bmlayer_p.h>

QT_BEGIN_NAMESPACE

class QJsonObject;

class LottieRenderer;
class BMShape;
class BMTrimPath;
class BMBasicTransform;

class BODYMOVIN_EXPORT BMImageLayer : public BMLayer
{
public:
    BMImageLayer() = default;
    explicit BMImageLayer(const BMImageLayer &other);
    BMImageLayer(const QJsonObject &definition);
    ~BMImageLayer() override;

    BMBase *clone() const override;

    void updateProperties(int frame) override;
    void render(LottieRenderer &render) const override;

protected:
    QList<int> m_maskProperties;

private:
    BMTrimPath *m_appliedTrim = nullptr;
};

QT_END_NAMESPACE

#endif // BMIMAGELAYER_P_H
