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

#ifndef BMFEEFORMSHAPE_P_H
#define BMFEEFORMSHAPE_P_H

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

#include <QPainterPath>
#include <QJsonArray>

#include <QtBodymovin/bmglobal.h>
#include <QtBodymovin/private/bmshape_p.h>
#include <QtBodymovin/private/bmtrimpath_p.h>
#include <QtBodymovin/private/lottierenderer_p.h>

QT_BEGIN_NAMESPACE

class QJsonObject;

class BODYMOVIN_EXPORT BMFreeFormShape : public BMShape
{
public:
    BMFreeFormShape();
    explicit BMFreeFormShape(const BMFreeFormShape &other);
    BMFreeFormShape(const QJsonObject &definition, BMBase *parent = nullptr);

    BMBase *clone() const override;

    void construct(const QJsonObject &definition);

    void updateProperties(int frame) override;
    void render(LottieRenderer &renderer) const override;

    bool acceptsTrim() const override;

protected:
    struct VertexInfo {
        BMProperty2D<QPointF> pos;
        BMProperty2D<QPointF> ci;
        BMProperty2D<QPointF> co;
    };

    void parseShapeKeyframes(QJsonObject &keyframes);
    void buildShape(const QJsonObject &keyframe);
    void buildShape(int frame);
    void parseEasedVertices(const QJsonObject &keyframe, int startFrame);

    QHash<int, QJsonObject> m_vertexMap;
    QList<VertexInfo> m_vertexList;
    QMap<int, bool> m_closedShape;

private:
    struct VertexBuildInfo
    {
        QJsonArray posKeyframes;
        QJsonArray ciKeyframes;
        QJsonArray coKeyframes;
    };

    void finalizeVertices();

    QMap<int, VertexBuildInfo*> m_vertexInfos;

    QJsonObject createKeyframe(QJsonArray startValue, QJsonArray endValue,
                               int startFrame, QJsonObject easingIn,
                               QJsonObject easingOut);
};

QT_END_NAMESPACE

#endif // BMFEEFORMSHAPE_P_H
