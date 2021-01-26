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

#ifndef BMGROUP_P_H
#define BMGROUP_P_H

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

#include <QJsonObject>
#include <QColor>

#include <QtBodymovin/private/bmshape_p.h>
#include <QtBodymovin/private/bmproperty_p.h>
#include <QtBodymovin/private/bmpathtrimmer_p.h>

QT_BEGIN_NAMESPACE

class BMFill;
class BMTrimPath;
class BMPathTrimmer;

class BODYMOVIN_EXPORT BMGroup : public BMShape
{
public:
    BMGroup() = default;
    BMGroup(const QJsonObject &definition, BMBase *parent = nullptr);

    BMBase *clone() const override;

    void construct(const QJsonObject& definition);

    void updateProperties(int frame) override;
    void render(LottieRenderer &renderer) const override;

    bool acceptsTrim() const override;
    void applyTrim(const BMTrimPath  &trimmer) override;
};

QT_END_NAMESPACE

#endif // BMGROUP_P_H
