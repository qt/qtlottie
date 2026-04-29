// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qlottietextlayer_p.h"

#include <QJsonArray>
#include <QJsonObject>

#include "qlottieconstants_p.h"
#include "qlottierenderer_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

namespace {

QPainterPath buildContour(const QJsonObject &shape)
{
    const QJsonArray bezierIn = shape.value("i"_L1).toArray();
    const QJsonArray bezierOut = shape.value("o"_L1).toArray();
    const QJsonArray vertices = shape.value("v"_L1).toArray();
    const bool closed = shape.value("c"_L1).toBool();

    QPainterPath path;
    if (vertices.count() < 2)
        return path;

    auto toPoint = [](const QJsonArray &arr, int idx) {
        return QPointF(arr.at(idx).toArray().at(0).toDouble(),
                       arr.at(idx).toArray().at(1).toDouble());
    };

    QPointF s = toPoint(vertices, 0);
    path.moveTo(s);

    for (int i = 0; i < vertices.count() - 1; ++i) {
        QPointF v = toPoint(vertices, i + 1);
        QPointF c1 = s + toPoint(bezierOut, i);
        QPointF c2 = v + toPoint(bezierIn, i + 1);
        path.cubicTo(c1, c2, v);
        s = v;
    }

    if (closed) {
        QPointF start = toPoint(vertices, 0);
        QPointF c1 = s + toPoint(bezierOut, vertices.count() - 1);
        QPointF c2 = start + toPoint(bezierIn, 0);
        path.cubicTo(c1, c2, start);
        path.closeSubpath();
    }

    return path;
}

QPainterPath buildGlyphPathFromData(const QJsonObject &data)
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);

    for (const QJsonValue &shapeVal : data.value("shapes"_L1).toArray()) {
        for (const QJsonValue &itemVal : shapeVal.toObject().value("it"_L1).toArray()) {
            const QJsonObject item = itemVal.toObject();
            if (item.value("ty"_L1).toString() == "sh"_L1) {
                const QJsonObject ks = item.value("ks"_L1).toObject();
                if (!ks.value("a"_L1).toInt())  // static (not animated)
                    path.addPath(buildContour(ks.value("k"_L1).toObject()));
            }
        }
    }

    return path;
}

QString charKey(const QChar &ch, const QString &fFamily)
{
    return QString(ch) + u'|' + fFamily;
}

} // namespace

QLottieTextLayer::QLottieTextLayer(const QLottieTextLayer &other)
    : QLottieLayer(other)
{
    m_textDocument = other.m_textDocument;
    m_chars = other.m_chars;
}

QLottieBase *QLottieTextLayer::clone() const
{
    return new QLottieTextLayer(*this);
}

void QLottieTextLayer::render(QLottieRenderer &renderer) const
{
    if (!m_isActive)
        return;

    renderer.saveState();
    applyLayerTransform(renderer);
    renderer.render(*this);
    renderer.finish(*this);
    renderer.restoreState();
}

int QLottieTextLayer::parse(const QJsonObject &definition)
{
    m_type = LOTTIE_LAYER_TEXT_IX;

    int ret = QLottieLayer::parse(definition);

    if (m_hidden)
        return 0;

    const QJsonArray keyframes = definition.value("t"_L1).toObject()
                                           .value("d"_L1).toObject()
                                           .value("k"_L1).toArray();

    if (!keyframes.isEmpty()) {
        const QJsonObject doc = keyframes.first().toObject().value("s"_L1).toObject();

        m_textDocument.text = doc.value("t"_L1).toString();
        m_textDocument.fontFamily = doc.value("f"_L1).toString();
        m_textDocument.fontSize = doc.value("s"_L1).toDouble(12.0);
        m_textDocument.justification = doc.value("j"_L1).toInt(0);
        m_textDocument.tracking = doc.value("tr"_L1).toDouble(0.0);

        const QJsonArray fc = doc.value("fc"_L1).toArray();
        if (fc.size() >= 3) {
            m_textDocument.fillColor = QColor::fromRgbF(
                fc.at(0).toDouble(),
                fc.at(1).toDouble(),
                fc.at(2).toDouble(),
                fc.size() >= 4 ? fc.at(3).toDouble() : 1.0);
        }
    }

    qCDebug(lcLottieQtLottieParser) << "QLottieTextLayer::parse()" << m_name;
    return ret;
}

void QLottieTextLayer::setCharData(const QMap<QString, QJsonObject> &chars)
{
    m_chars.clear();
    for (auto it = chars.constBegin(); it != chars.constEnd(); ++it) {
        CharInfo info;
        info.advanceWidth = it.value().value("w"_L1).toDouble();
        info.glyphPath = buildGlyphPathFromData(it.value().value("data"_L1).toObject());
        m_chars.insert(it.key(), info);
    }
}

QList<QPainterPath> QLottieTextLayer::buildGlyphPaths() const
{
    if (m_textDocument.text.isEmpty() || m_chars.isEmpty())
        return {};

    const qreal scale = m_textDocument.fontSize / 100.0;
    const qreal trackingPx = m_textDocument.tracking * scale / 10.0;

    qreal totalWidth = 0;
    for (const QChar &ch : m_textDocument.text) {
        auto it = m_chars.constFind(charKey(ch, m_textDocument.fontFamily));
        if (it != m_chars.constEnd())
            totalWidth += it->advanceWidth * scale + trackingPx;
    }

    qreal x = 0;
    if (m_textDocument.justification == 1) // right align
        x = -totalWidth;
    else if (m_textDocument.justification == 2) // center align
        x = -totalWidth / 2.0;

    QList<QPainterPath> result;
    for (const QChar &ch : m_textDocument.text) {
        auto it = m_chars.constFind(charKey(ch, m_textDocument.fontFamily));
        if (it != m_chars.constEnd()) {
            if (!it->glyphPath.isEmpty()) {
                QTransform t;
                t.translate(x, 0);
                t.scale(scale, scale);
                result.append(t.map(it->glyphPath));
            }
            x += it->advanceWidth * scale + trackingPx;
        }
    }

    return result;
}

QT_END_NAMESPACE
