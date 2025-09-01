// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottiebasictransform_p.h"

#include <QJsonObject>
#include <QString>

#include "qlottieconstants_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottieBasicTransform::QLottieBasicTransform(const QLottieBasicTransform &other)
    : QLottieShape(other)
{
    m_direction = other.m_direction;
    m_anchorPoint = other.m_anchorPoint;
    m_splitPosition = other.m_splitPosition;
    m_position = other.m_position;
    m_xPos = other.m_xPos;
    m_yPos = other.m_yPos;
    m_scale = other.m_scale;
    m_rotation = other.m_rotation;
    m_opacity = other.m_opacity;
}

QLottieBasicTransform::QLottieBasicTransform(QLottieBase *parent)
{
    setParent(parent);
}

QLottieBase *QLottieBasicTransform::clone() const
{
    return new QLottieBasicTransform(*this);
}

int QLottieBasicTransform::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);

    qCDebug(lcLottieQtLottieParser)
            << "QLottieBasicTransform::parse():" << m_name;

    if (definition.contains(u"a"_s)) {
        QJsonObject anchors = definition.value(u"a"_s).toObject();
        anchors = resolveExpression(anchors);
        m_anchorPoint.construct(anchors);
    }

    if (definition.contains(u"p"_s)) {
        QJsonObject position = definition.value(u"p"_s).toObject();
        if (position.contains(u"s"_s)) {
            if (!checkRequiredKey(position, u"Transform"_s, {u"x"_s, u"y"_s}, m_name))
                return -1;

            QJsonObject posX = position.value(u"x"_s).toObject();
            posX = resolveExpression(posX);
            m_xPos.construct(posX);

            QJsonObject posY = position.value(u"y"_s).toObject();
            posY = resolveExpression(posY);
            m_yPos.construct(posY);

            m_splitPosition = true;
        } else {
            QJsonObject position = definition.value(u"p"_s).toObject();
            position = resolveExpression(position);
            m_position.construct(position);
        }
    }

    if (definition.contains(u"s"_s)) {
        QJsonObject scale = definition.value(u"s"_s).toObject();
        scale = resolveExpression(scale);
        m_scale.construct(scale);
    } else {
        m_scale.setValue(QPointF(100, 100));
    }

    if (definition.contains(u"r"_s)) {
        QJsonObject rotation = definition.value(u"r"_s).toObject();
        rotation = resolveExpression(rotation);
        m_rotation.construct(rotation);
    } else {
        m_rotation.setValue(0);
    }

    // If this is the base class for QLottieRepeaterTransform,
    // opacity is not present
    if (definition.contains(u"o"_s)) {
        QJsonObject opacity = definition.value(u"o"_s).toObject();
        opacity = resolveExpression(opacity);
        m_opacity.construct(opacity);
    } else {
        m_opacity.setValue(100);
    }

    return 0;
}

void QLottieBasicTransform::updateProperties(int frame)
{
    if (m_splitPosition) {
        m_xPos.update(frame);
        m_yPos.update(frame);
    } else
        m_position.update(frame);
    m_anchorPoint.update(frame);
    m_scale.update(frame);
    m_rotation.update(frame);
    m_opacity.update(frame);
}

void QLottieBasicTransform::render(QLottieRenderer &renderer) const
{
    renderer.render(*this);
}

QPointF QLottieBasicTransform::anchorPoint() const
{
    return m_anchorPoint.value();
}

QPointF QLottieBasicTransform::position() const
{
    if (m_splitPosition)
        return QPointF(m_xPos.value(), m_yPos.value());
    else
        return m_position.value();
}

QPointF QLottieBasicTransform::scale() const
{
    // Scale the value to 0..1 to be suitable for Qt
    return m_scale.value() / 100.0;
}

qreal QLottieBasicTransform::rotation() const
{
    return m_rotation.value();
}

qreal QLottieBasicTransform::opacity() const
{
    // Scale the value to 0..1 to be suitable for Qt
    return m_opacity.value() / 100.0;
}

QT_END_NAMESPACE
