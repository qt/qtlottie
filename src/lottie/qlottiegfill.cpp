// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottiegfill_p.h"

#include <QLinearGradient>
#include <QRadialGradient>
#include <QtMath>
#include <QColor>
#include <QString>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottieGFill::QLottieGFill(const QLottieGFill &other)
    : QLottieShape(other)
{
    if (m_hidden)
        return;

    m_opacity = other.m_opacity;
    m_startPoint = other.m_startPoint;
    m_endPoint = other.m_endPoint;
    m_highlightLength = other.m_highlightLength;
    m_highlightAngle = other.m_highlightAngle;
    m_colors = other.m_colors;
    if (other.gradientType() == QGradient::LinearGradient)
        m_gradient = new QLinearGradient;
    else if (other.gradientType() == QGradient::RadialGradient)
        m_gradient = new QRadialGradient;
    else {
        Q_UNREACHABLE();
    }
    m_fillRule = other.m_fillRule;
}

QLottieGFill::~QLottieGFill()
{
    if (m_gradient)
        delete m_gradient;
}

QLottieBase *QLottieGFill::clone() const
{
    return new QLottieGFill(*this);
}

QLottieGFill::QLottieGFill(QLottieBase *parent)
{
    setParent(parent);
}

void QLottieGFill::updateProperties(int frame)
{
    QGradient::Type type = gradientType();
    if (type != QGradient::LinearGradient &&
        type != QGradient::RadialGradient)
        return;

    m_startPoint.update(frame);
    m_endPoint.update(frame);
    m_highlightLength.update(frame);
    m_highlightAngle.update(frame);
    m_opacity.update(frame);
    QHash<qreal, QLottieProperty4D<QVector4D>>::iterator colorIt = m_colors.begin();
    while (colorIt != m_colors.end()) {
        (*colorIt).update(frame);
        ++colorIt;
    }

    setGradient();
}

void QLottieGFill::render(QLottieRenderer &renderer) const
{
    renderer.render(*this);
}

int QLottieGFill::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottieGFill::parse():" << m_name;

    if (!checkRequiredKeys(definition, "Gradient"_L1, { "s"_L1, "e"_L1, "g"_L1, "t"_L1, "o"_L1 }, m_name))
        return -1;

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
    if (!checkRequiredKeys(color, "Gradient"_L1, { "p"_L1, "k"_L1 }, m_name))
        return -1;

    int elementCount = color.value("p"_L1).toInt();
    QJsonObject stops = color.value("k"_L1).toObject();
    bool isAnimated = stops.value("a"_L1).toVariant().toBool();
    if (!checkRequiredKeys(stops, "Gradient"_L1, { "a"_L1, "k"_L1 }, m_name))
        return -1;

    if (!isAnimated) {
        QJsonArray colorArr = stops.value("k"_L1).toArray();
        for (int i = 0; i < (elementCount * 4); i += 4) {
            // p denotes the color stop percentage
            QVector4D colorVec;
            colorVec[0] = colorArr.at(i + 1).toVariant().toFloat();
            colorVec[1] = colorArr.at(i + 2).toVariant().toFloat();
            colorVec[2] = colorArr.at(i + 3).toVariant().toFloat();
            // Set gradient stop position into w of the vector
            colorVec[3] = 1.0f;
            QLottieProperty4D<QVector4D> colorPos;
            colorPos.setValue(colorVec);
            qreal pos = colorArr.at(i + 0).toVariant().toFloat();
            m_colors[pos] = colorPos;
        }
        QList<qreal> keys = m_colors.keys();
        std::sort(keys.begin(), keys.end());
        int keyIndex = 0;
        for (int i = (elementCount * 4); i < colorArr.size(); i+= 2) {
            qreal pos = colorArr.at(i).toVariant().toFloat();
            qreal opacity = colorArr.at(i + 1).toVariant().toFloat();
            if (m_colors.contains(pos)) {
                QLottieProperty4D<QVector4D> colorVec = m_colors.value(pos);
                QVector4D color = colorVec.value();
                color[3] = opacity;
                colorVec.setValue(color);
                m_colors[pos] = colorVec;
                keyIndex++;
            } else {
                qreal pos0;
                qreal pos1;
                if (keyIndex == 0) {
                    pos0 = keys.value(0);
                    pos1 = keys.value(1);
                } else {
                    pos0 = keys.value(keyIndex-1);
                    pos1 = keys.value(keyIndex);
                }

                if (pos1 > pos) {
                    QVector4D col0 = m_colors[pos0].value();
                    QVector4D col1 = m_colors[pos1].value();
                    qreal ratio = (pos - pos0) / (pos1 - pos0);
                    qreal r = col0[0] + ratio * (col1[0] - col0[0]);
                    qreal g = col0[1] + ratio * (col1[1] - col0[1]);
                    qreal b = col0[2] + ratio * (col1[2] - col0[2]);
                    QVector4D color = QVector4D(r, g, b, opacity);
                    QLottieProperty4D<QVector4D> colorPos;
                    colorPos.setValue(color);
                    m_colors[pos] = colorPos;
                } else {
                    QLottieProperty4D<QVector4D> colorVec = m_colors.value(pos1);
                    QVector4D color = colorVec.value();
                    qreal opa0 = m_colors[pos1].value()[3];
                    qreal ratio = (pos1 - pos0) / (pos - pos0);
                    qreal opa = opa0 + ratio * (opacity - opa0);
                    color[3] = opa;
                    colorVec.setValue(color);
                    m_colors[pos1] = colorVec;
                    keyIndex++;
                }
            }
        }
    } else {
        qCInfo(lcLottieQtLottieParser) << "Animated gradient is not supported";
    }

    QJsonObject opacity = definition.value("o"_L1).toObject();
    opacity = resolveExpression(opacity);
    m_opacity.construct(opacity);

    QJsonObject startPoint = definition.value("s"_L1).toObject();
    startPoint = resolveExpression(startPoint);
    m_startPoint.construct(startPoint);

    QJsonObject endPoint = definition.value("e"_L1).toObject();
    endPoint = resolveExpression(endPoint);
    m_endPoint.construct(endPoint);

    QJsonObject highlight = definition.value("h"_L1).toObject();
    m_highlightLength.construct(highlight);

    QJsonObject angle = definition.value("a"_L1).toObject();
    angle = resolveExpression(angle);
    m_highlightAngle.construct(angle);

    const int fillValue = definition.value("r"_L1).toInt();
    m_fillRule = (fillValue == 2) ? Qt::OddEvenFill : Qt::WindingFill;

    return 0;
}

QGradient *QLottieGFill::value() const
{
    return m_gradient;
}

QGradient::Type QLottieGFill::gradientType() const
{
    if (m_gradient)
        return m_gradient->type();
    else
        return QGradient::NoGradient;
}

QPointF QLottieGFill::startPoint() const
{
    return m_startPoint.value();
}

QPointF QLottieGFill::endPoint() const
{
    return m_endPoint.value();
}

qreal QLottieGFill::highlightLength() const
{
    return m_highlightLength.value();
}

qreal QLottieGFill::highlightAngle() const
{
    return m_highlightAngle.value();
}

qreal QLottieGFill::opacity() const
{
    return m_opacity.value();
}

Qt::FillRule QLottieGFill::fillRule() const
{
    return m_fillRule;
}

void QLottieGFill::setGradient()
{
    QHash<qreal, QLottieProperty4D<QVector4D>>::iterator colorIt = m_colors.begin();
    while (colorIt != m_colors.end()) {
        QVector4D colorPos = (*colorIt).value();
        qreal pos = colorIt.key();
        qreal opacity = m_opacity.value() / 100.0;
        opacity *= static_cast<qreal>(colorPos[3]);
        QColor color;
        color.setRedF(static_cast<qreal>(colorPos[0]));
        color.setGreenF(static_cast<qreal>(colorPos[1]));
        color.setBlueF(static_cast<qreal>(colorPos[2]));
        color.setAlphaF(opacity);
        m_gradient->setColorAt(pos, color);
        ++colorIt;
    }

    switch (gradientType()) {
    case QGradient::LinearGradient:
    {
        QLinearGradient *g = static_cast<QLinearGradient*>(m_gradient);
        g->setStart(m_startPoint.value());
        g->setFinalStop(m_endPoint.value());
        break;
    }
    case QGradient::RadialGradient:
    {
        QRadialGradient *g = static_cast<QRadialGradient*>(m_gradient);
        QLineF radLine(m_startPoint.value(), m_endPoint.value());
        g->setCenter(radLine.p1());
        g->setRadius(radLine.length());
        radLine.setAngle(radLine.angle() - m_highlightAngle.value());
        // QRadialGradient needs focalPoint to be inside (and not on) radius circle
        qreal radFraction = qMin((m_highlightLength.value() / 100.0), 0.999);
        g->setFocalPoint(radLine.pointAt(radFraction));
        break;
    }
    default:
        break;
    }
}

QT_END_NAMESPACE
