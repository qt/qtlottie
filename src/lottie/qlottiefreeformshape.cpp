// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottiefreeformshape_p.h"

#include <QJsonObject>
#include <QString>

#include "qlottietrimpath_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottieFreeFormShape::QLottieFreeFormShape(const QLottieFreeFormShape &other)
    : QLottieShape(other)
{
    m_vertexList = other.m_vertexList;
    m_closedShape = other.m_closedShape;
    m_vertexMap = other.m_vertexMap;
}

QLottieFreeFormShape::QLottieFreeFormShape(QLottieBase *parent)
{
    setParent(parent);
}

QLottieBase *QLottieFreeFormShape::clone() const
{
    return new QLottieFreeFormShape(*this);
}

int QLottieFreeFormShape::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottieFreeFormShape::parse():" << m_name;

    m_direction = definition.value(u"d"_s).toVariant().toInt();

    if (!checkRequiredKey(definition, QStringLiteral("Shape"), {u"ks"_s}, m_name))
        return -1;
    QJsonObject vertexObj = definition.value(u"ks"_s).toObject();
    if (!checkRequiredKey(vertexObj, QStringLiteral("Shape"), {u"k"_s}, m_name))
        return -1;

    if (vertexObj.value(u"a"_s).toInt())
        parseShapeKeyframes(vertexObj);
    else
        buildShape(vertexObj.value(u"k"_s).toObject());

    return 0;
}

void QLottieFreeFormShape::updateProperties(int frame)
{
    if (m_vertexMap.size()) {
        QJsonObject keyframe = m_vertexMap.value(frame);
        // If this frame is a keyframe, so values must be updated
        if (!keyframe.isEmpty())
            buildShape(keyframe.value(u"s"_s).toArray().at(0).toObject());
    } else {
        for (int i =0; i < m_vertexList.size(); i++) {
            VertexInfo vi = m_vertexList.at(i);
            vi.pos.update(frame);
            vi.ci.update(frame);
            vi.co.update(frame);
            m_vertexList.replace(i, vi);
        }
        buildShape(frame);
    }
}

void QLottieFreeFormShape::render(QLottieRenderer &renderer) const
{
    renderer.render(*this);
}

bool QLottieFreeFormShape::acceptsTrim() const
{
    return true;
}

bool QLottieFreeFormShape::isAnimated() const
{
    return !m_vertexList.isEmpty();
}

void QLottieFreeFormShape::parseShapeKeyframes(QJsonObject &keyframes)
{
    QJsonArray vertexKeyframes = keyframes.value(u"k"_s).toArray();
    for (int i = 0; i < vertexKeyframes.count(); i++) {
        QJsonObject keyframe = vertexKeyframes.at(i).toObject();
        if (keyframe.value(u"h"_s).toInt()) {
            m_vertexMap.insert(keyframe.value(u"t"_s).toVariant().toInt(), keyframe);
        } else
            parseEasedVertices(keyframe, keyframe.value(u"t"_s).toVariant().toInt());
    }
    if (m_vertexInfos.size())
        finalizeVertices();
}

void QLottieFreeFormShape::buildShape(const QJsonObject &shape)
{
    m_path.clear();

    bool needToClose = shape.value(u"c"_s).toBool();
    QJsonArray bezierIn = shape.value(u"i"_s).toArray();
    QJsonArray bezierOut = shape.value(u"o"_s).toArray();
    QJsonArray vertices = shape.value(u"v"_s).toArray();

    // If there are less than two vertices, cannot make a bezier curve
    if (vertices.count() < 2)
        return;

    QPointF s(vertices.at(0).toArray().at(0).toDouble(),
              vertices.at(0).toArray().at(1).toDouble());
    QPointF s0(s);

    m_path.moveTo(s);
    int i=0;

    while (i < vertices.count() - 1) {
        QPointF v = QPointF(vertices.at(i + 1).toArray().at(0).toDouble(),
                            vertices.at(i + 1).toArray().at(1).toDouble());
        QPointF c1 = QPointF(bezierOut.at(i).toArray().at(0).toDouble(),
                             bezierOut.at(i).toArray().at(1).toDouble());
        QPointF c2 = QPointF(bezierIn.at(i + 1).toArray().at(0).toDouble(),
                             bezierIn.at(i + 1).toArray().at(1).toDouble());
        c1 += s;
        c2 += v;

        m_path.cubicTo(c1, c2, v);

        s = v;
        i++;
    }

    if (needToClose) {
        QPointF v = s0;
        QPointF c1 = QPointF(bezierOut.at(i).toArray().at(0).toDouble(),
                             bezierOut.at(i).toArray().at(1).toDouble());
        QPointF c2 = QPointF(bezierIn.at(0).toArray().at(0).toDouble(),
                             bezierIn.at(0).toArray().at(1).toDouble());
        c1 += s;
        c2 += v;

        m_path.cubicTo(c1, c2, v);
    }

    m_path.setFillRule(Qt::WindingFill);

    if (hasReversedDirection())
        m_path = m_path.toReversed();
}

void QLottieFreeFormShape::buildShape(int frame)
{
    if (m_closedShape.size()) {
        m_path.clear();

        auto it = m_closedShape.constBegin();
        bool found = false;

        if (frame <= it.key())
            found = true;
        else {
            while (it != m_closedShape.constEnd()) {
                if (it.key() <= frame) {
                    found = true;
                    break;
                }
                ++it;
            }
        }

        bool needToClose = false;
        if (found)
            needToClose = (*it);

        // If there are less than two vertices, cannot make a bezier curve
        if (m_vertexList.size() < 2)
            return;

        QPointF s(m_vertexList.at(0).pos.value());
        QPointF s0(s);

        m_path.moveTo(s);
        int i = 0;

        while (i < m_vertexList.size() - 1) {
            QPointF v = m_vertexList.at(i + 1).pos.value();
            QPointF c1 = m_vertexList.at(i).co.value();
            QPointF c2 = m_vertexList.at(i + 1).ci.value();
            c1 += s;
            c2 += v;

            m_path.cubicTo(c1, c2, v);

            s = v;
            i++;
        }

        if (needToClose) {
            QPointF v = s0;
            QPointF c1 = m_vertexList.at(i).co.value();
            QPointF c2 = m_vertexList.at(0).ci.value();
            c1 += s;
            c2 += v;

            m_path.cubicTo(c1, c2, v);
        }

        m_path.setFillRule(Qt::WindingFill);

        if (hasReversedDirection())
            m_path = m_path.toReversed();
    }
}

void QLottieFreeFormShape::parseEasedVertices(const QJsonObject &keyframe, int startFrame)
{
    Q_UNUSED(startFrame);

    QJsonObject startValue = keyframe.value(u"s"_s).toArray().at(0).toObject();
    QJsonObject endValue = keyframe.value(u"e"_s).toArray().at(0).toObject();
    bool closedPathAtStart = keyframe.value(u"s"_s).toArray().at(0).toObject().value(u"c"_s).toBool();
    //bool closedPathAtEnd = keyframe.value(u"e"_s).toArray().at(0).toObject().value(u"c"_s).toBool();
    QJsonArray startVertices = startValue.value(u"v"_s).toArray();
    QJsonArray startBezierIn = startValue.value(u"i"_s).toArray();
    QJsonArray startBezierOut = startValue.value(u"o"_s).toArray();
    QJsonArray endVertices = endValue.value(u"v"_s).toArray();
    QJsonArray endBezierIn = endValue.value(u"i"_s).toArray();
    QJsonArray endBezierOut = endValue.value(u"o"_s).toArray();
    QJsonObject easingIn = keyframe.value(u"i"_s).toObject();
    QJsonObject easingOut = keyframe.value(u"o"_s).toObject();

    // if there are no vertices for this keyframe, they keyframe
    // is the last one, and it must be processed differently
    if (!startVertices.isEmpty()) {
        for (int i = 0; i < startVertices.count(); i++) {
            VertexBuildInfo *buildInfo = m_vertexInfos.value(i, nullptr);
            if (!buildInfo) {
                buildInfo = new VertexBuildInfo;
                m_vertexInfos.insert(i, buildInfo);
            }
            QJsonObject posKf = createKeyframe(startVertices.at(i).toArray(),
                                            endVertices.at(i).toArray(),
                                            startFrame, easingIn, easingOut);
            buildInfo->posKeyframes.push_back(posKf);

            QJsonObject ciKf = createKeyframe(startBezierIn.at(i).toArray(),
                                            endBezierIn.at(i).toArray(),
                                            startFrame, easingIn, easingOut);
            buildInfo->ciKeyframes.push_back(ciKf);

            QJsonObject coKf = createKeyframe(startBezierOut.at(i).toArray(),
                                            endBezierOut.at(i).toArray(),
                                            startFrame, easingIn, easingOut);
            buildInfo->coKeyframes.push_back(coKf);

            m_closedShape.insert(startFrame, closedPathAtStart);
        }
    } else {
        // Last keyframe

        int vertexCount = m_vertexInfos.size();
        for (int i = 0; i < vertexCount; i++) {
            VertexBuildInfo *buildInfo = m_vertexInfos.value(i, nullptr);
            if (!buildInfo) {
                buildInfo = new VertexBuildInfo;
                m_vertexInfos.insert(i, buildInfo);
            }
            QJsonObject posKf;
            posKf.insert(u"t"_s, startFrame);
            buildInfo->posKeyframes.push_back(posKf);

            QJsonObject ciKf;
            ciKf.insert(u"t"_s, startFrame);
            buildInfo->ciKeyframes.push_back(ciKf);

            QJsonObject coKf;
            coKf.insert(u"t"_s, startFrame);
            buildInfo->coKeyframes.push_back(coKf);

            m_closedShape.insert(startFrame, false);
        }
    }
}

void QLottieFreeFormShape::finalizeVertices()
{

    for (int i = 0; i < m_vertexInfos.size(); i++) {
        QJsonObject posObj;
        posObj.insert(u"a"_s, 1);
        posObj.insert(u"k"_s, m_vertexInfos.value(i)->posKeyframes);

        QJsonObject ciObj;
        ciObj.insert(u"a"_s, 1);
        ciObj.insert(u"k"_s, m_vertexInfos.value(i)->ciKeyframes);

        QJsonObject coObj;
        coObj.insert(u"a"_s, 1);
        coObj.insert(u"k"_s, m_vertexInfos.value(i)->coKeyframes);

        VertexInfo vertexInfo;
        vertexInfo.pos.construct(posObj);
        vertexInfo.ci.construct(ciObj);
        vertexInfo.co.construct(coObj);
        m_vertexList.push_back(vertexInfo);
    }
    qDeleteAll(m_vertexInfos);
}

QJsonObject QLottieFreeFormShape::createKeyframe(QJsonArray startValue, QJsonArray endValue,
                                            int startFrame, QJsonObject easingIn,
                                            QJsonObject easingOut)
{
    QJsonObject keyframe;
    keyframe.insert(u"t"_s, startFrame);
    keyframe.insert(u"s"_s, startValue);
    keyframe.insert(u"e"_s, endValue);
    keyframe.insert(u"i"_s, easingIn);
    keyframe.insert(u"o"_s, easingOut);
    return keyframe;
}

QT_END_NAMESPACE
