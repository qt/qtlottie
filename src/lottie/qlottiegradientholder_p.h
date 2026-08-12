// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEGRADIENTHOLDER_P_H
#define QLOTTIEGRADIENTHOLDER_P_H

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

#include <QtLottie/private/qlottiespatialproperty_p.h>
#include <QtLottie/private/qlottiestroke_p.h>

QT_BEGIN_NAMESPACE

class Q_LOTTIE_EXPORT QLottieGradientHolderContent
{
public:
    QGradient *value() const;
    QGradient::Type gradientType() const;
    QPointF startPoint() const;
    QPointF endPoint() const;
    qreal highlightLength() const;
    qreal highlightAngle() const;

protected:
    void setGradient(qreal opacity);
    void updateGradientProperties(int frame);

    QGradient *m_gradient = nullptr;
    QLottieSpatialProperty m_startPoint;
    QLottieSpatialProperty m_endPoint;
    QLottieProperty<qreal> m_highlightLength;
    QLottieProperty<qreal> m_highlightAngle;
    QHash<qreal, QLottieProperty4D<QVector4D>> m_colors;
};

template <typename T>
class QLottieGradientHolder: public T, public QLottieGradientHolderContent
{
public:
    QLottieGradientHolder(QLottieBase *parent);
    QLottieGradientHolder(const QLottieGradientHolder<T> &other);
    ~QLottieGradientHolder() override;

protected:
    bool parseGradientProperties(const QJsonObject &definition);
};

template <typename T>
QLottieGradientHolder<T>::QLottieGradientHolder(QLottieBase *parent)
{
    T::setParent(parent);
}

template <typename T>
QLottieGradientHolder<T>::QLottieGradientHolder(const QLottieGradientHolder<T> &other)
    : T(other)
{
    if (T::m_hidden)
        return;

    m_startPoint = other.m_startPoint;
    m_endPoint = other.m_endPoint;
    m_highlightLength = other.m_highlightLength;
    m_highlightAngle = other.m_highlightAngle;
    m_colors = other.m_colors;

    if (other.gradientType() == QGradient::LinearGradient)
        m_gradient = new QLinearGradient;
    else if (other.gradientType() == QGradient::RadialGradient)
        m_gradient = new QRadialGradient;
    else
        Q_UNREACHABLE();
}

template <typename T>
QLottieGradientHolder<T>::~QLottieGradientHolder()
{
    delete m_gradient;
}

template <typename T>
bool QLottieGradientHolder<T>::parseGradientProperties(const QJsonObject &definition)
{
    if (!T::checkRequiredKeys(definition, "Gradient"_L1, { "s"_L1, "e"_L1, "g"_L1, "t"_L1 }, T::m_name))
        return false;

    int type = definition.value("t"_L1).toVariant().toInt();
    switch (type) {
    case 1:
        m_gradient = new QLinearGradient;
        break;
    case 2:
        m_gradient = new QRadialGradient;
        break;
    default:
        qCWarning(lcLottieQtLottieParser) << "Unknown gradient fill type";
    }

    QJsonObject color = definition.value("g"_L1).toObject();
    if (!T::checkRequiredKeys(color, "Gradient"_L1, { "p"_L1, "k"_L1 }, T::m_name))
        return false;

    int elementCount = color.value("p"_L1).toInt();
    if (elementCount <= 0) {
        qCInfo(lcLottieQtLottieParser) << "Empty gradient enountered";
        return true;
    }

    QJsonObject stops = color.value("k"_L1).toObject();
    bool isAnimated = stops.value("a"_L1).toVariant().toBool();
    if (!T::checkRequiredKeys(stops, "Gradient"_L1, { "a"_L1, "k"_L1 }, T::m_name))
        return false;

    if (!isAnimated) {
        QMap<qreal, QVector4D> colorStops;
        QJsonArray colorArr = stops.value("k"_L1).toArray();
        if (colorArr.size() < elementCount * 4) {
            qCWarning(lcLottieQtLottieParser) << "Gradient color array too short";
            return false;
        }
        for (int i = 0; i < (elementCount * 4); i += 4) {
            // p denotes the color stop percentage
            QVector4D colorVec;
            colorVec[0] = colorArr.at(i + 1).toVariant().toFloat();
            colorVec[1] = colorArr.at(i + 2).toVariant().toFloat();
            colorVec[2] = colorArr.at(i + 3).toVariant().toFloat();
            colorVec[3] = 1.0f;
            QLottieProperty4D<QVector4D> colorPos;
            colorPos.setValue(colorVec);
            // The stop position is found in the first element of the vector
            qreal pos = colorArr.at(i + 0).toVariant().toFloat();
            m_colors[pos] = colorPos;
            colorStops.insert(pos, colorVec);
        }

        if (colorArr.size() > (elementCount * 4)) {
            // The gradient has opacity stops; handle them
            QMap<qreal, qreal> opacityStops;
            for (int i = (elementCount * 4); i < colorArr.size(); i += 2) {
                qreal pos = colorArr.at(i).toVariant().toFloat();
                qreal opacity = colorArr.at(i + 1).toVariant().toFloat();
                opacityStops.insert(pos, opacity);
            }
            QMap<qreal, qreal> uniqueOpacityStops(opacityStops);

            // First: add opacity to all existing color stops
            for (auto [pos, color] : colorStops.asKeyValueRange()) {
                qreal opacity = 1.0;
                if (pos <= opacityStops.firstKey() || opacityStops.size() == 1) {
                    opacity = opacityStops.first();
                } else if (pos >= opacityStops.lastKey()) {
                    opacity = opacityStops.last();
                } else {
                    auto it2 = opacityStops.lowerBound(pos);
                    Q_ASSERT(it2 != opacityStops.cbegin());
                    auto it1(it2);
                    it1--;
                    qreal ratio = (pos - it1.key()) / (it2.key() - it1.key());
                    opacity = it1.value() + ratio * (it2.value() - it1.value());
                }
                color[3] = opacity;
                m_colors[pos].setValue(color);
                uniqueOpacityStops.remove(pos); // will remove if position present, otherwise noop
            }

            // Second: add new color stops for any unique opacity stop positions
            for (auto [pos, opacity] : uniqueOpacityStops.asKeyValueRange()) {
                QVector4D color;
                if (pos <= colorStops.firstKey() || colorStops.size() == 1) {
                    color = colorStops.first();
                } else if (pos >= colorStops.lastKey()) {
                    color = colorStops.last();
                } else {
                    auto it2 = colorStops.lowerBound(pos);
                    Q_ASSERT(it2 != colorStops.cbegin());
                    auto it1(it2);
                    it1--;
                    qreal ratio = (pos - it1.key()) / (it2.key() - it1.key());
                    color = it1.value() + ratio * (it2.value() - it1.value());
                }
                color[3] = opacity;
                m_colors[pos].setValue(color);
            }
        }
    } else {
        qCInfo(lcLottieQtLottieParser) << "Animated gradient is not supported";
    }

    QJsonObject startPoint = definition.value("s"_L1).toObject();
    startPoint = T::resolveExpression(startPoint);
    m_startPoint.construct(startPoint);

    QJsonObject endPoint = definition.value("e"_L1).toObject();
    endPoint = T::resolveExpression(endPoint);
    m_endPoint.construct(endPoint);

    QJsonObject highlight = definition.value("h"_L1).toObject();
    m_highlightLength.construct(highlight);

    QJsonObject angle = definition.value("a"_L1).toObject();
    angle = T::resolveExpression(angle);
    m_highlightAngle.construct(angle);

    return true;
}

QT_END_NAMESPACE

#endif // QLOTTIEGRADIENTHOLDER_P_H
