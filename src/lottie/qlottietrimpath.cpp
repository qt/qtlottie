// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottietrimpath_p.h"

#include <QtGlobal>
#include <private/qpainterpath_p.h>
#include <private/qbezier_p.h>

#include "qlottieconstants_p.h"
#include "qtrimpath_p.h"

QLottieQTrimPath::QLottieQTrimPath()
{
    m_appliedTrim = this;
}

QLottieQTrimPath::QLottieQTrimPath(const QJsonObject &definition, const QVersionNumber &version, QLottieBase *parent)
{
    m_appliedTrim = this;

    setParent(parent);
    construct(definition, version);
}

QLottieQTrimPath::QLottieQTrimPath(const QLottieQTrimPath &other)
    : QLottieShape(other)
{
    m_start = other.m_start;
    m_end = other.m_end;
    m_offset = other.m_offset;
    m_simultaneous = other.m_simultaneous;
}

QLottieBase *QLottieQTrimPath::clone() const
{
    return new QLottieQTrimPath(*this);
}

void QLottieQTrimPath::construct(const QJsonObject &definition, const QVersionNumber &version)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return;

    qCDebug(lcLottieQtLottieParser) << "QLottieQTrimPath::construct():" << m_name;

    QJsonObject start = definition.value(QLatin1String("s")).toObject();
    start = resolveExpression(start);
    m_start.construct(start, version);

    QJsonObject end = definition.value(QLatin1String("e")).toObject();
    end = resolveExpression(end);
    m_end.construct(end, version);

    QJsonObject offset = definition.value(QLatin1String("o")).toObject();
    offset = resolveExpression(offset);
    m_offset.construct(offset, version);

    int simultaneous = true;
    if (definition.contains(QLatin1String("m"))) {
        simultaneous = definition.value(QLatin1String("m")).toInt();
    }
    m_simultaneous = (simultaneous == 1);

    if (strcmp(qgetenv("QLOTTIE_FORCE_TRIM_MODE"), "simultaneous") == 0) {
        qCDebug(lcLottieQtLottieRender) << "Forcing trim mode to Simultaneous";
        m_simultaneous = true;
    } else if (strcmp(qgetenv("QLOTTIE_FORCE_TRIM_MODE"), "individual") == 0) {
        qCDebug(lcLottieQtLottieRender) << "Forcing trim mode to Individual";
        m_simultaneous = false;
    }
}

void QLottieQTrimPath::updateProperties(int frame)
{
    m_start.update(frame);
    m_end.update(frame);
    m_offset.update(frame);

    qCDebug(lcLottieQtLottieUpdate) << name() << frame << m_start.value()
                                       << m_end.value() << m_offset.value();

    QLottieShape::updateProperties(frame);
}

void QLottieQTrimPath::render(QLottieRenderer &renderer) const
{
    if (m_appliedTrim) {
        if (m_appliedTrim->simultaneous())
            renderer.setTrimmingState(QLottieRenderer::Simultaneous);
        else
            renderer.setTrimmingState(QLottieRenderer::Individual);
    } else
        renderer.setTrimmingState(QLottieRenderer::Off);

    renderer.render(*this);
}

bool QLottieQTrimPath::acceptsTrim() const
{
    return true;
}

void QLottieQTrimPath::applyTrim(const QLottieQTrimPath &other)
{
     qCDebug(lcLottieQtLottieUpdate) << "Join trim paths:"
                                        << other.name() << "into:" << name();

    m_name = m_name + QStringLiteral(" & ") + other.name();
    qreal newStart = other.start() + (m_start.value() / 100.0) *
            (other.end() - other.start());
    qreal newEnd = other.start() + (m_end.value() / 100.0) *
            (other.end() - other.start());

    m_start.setValue(newStart);
    m_end.setValue(newEnd);
    m_offset.setValue(m_offset.value() + other.offset());
}

qreal QLottieQTrimPath::start() const
{
    return m_start.value();
}

qreal QLottieQTrimPath::end() const
{
    return m_end.value();
}

qreal QLottieQTrimPath::offset() const
{
    return m_offset.value();
}

bool QLottieQTrimPath::simultaneous() const
{
    return m_simultaneous;
}

QPainterPath QLottieQTrimPath::trim(const QPainterPath &path) const
{
    QTrimPath trimmer;
    trimmer.setPath(path);
    qreal offset = m_offset.value() / 360.0;
    qreal start = m_start.value() / 100.0;
    qreal end = m_end.value() / 100.0;
    QPainterPath trimmedPath;
    if (!qFuzzyIsNull(start - end))
        trimmedPath = trimmer.trimmed(start, end, offset);
    return trimmedPath;
}
