// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QJsonArray>
#include <QJsonObject>
#include <QLoggingCategory>
#include "qlottieconstants_p.h"
#include "qlottiebase_p.h"
#include "qlottieprecomposition_p.h"
#include "qlottielayer_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottiePrecomposition::QLottiePrecomposition()
{
    m_type = LOTTIE_PRECOMPOSITION_IX;
}

QLottiePrecomposition::QLottiePrecomposition(const QLottiePrecomposition &other)
    : QLottieBase(other)
{
    m_refId = other.m_refId;
}

QLottieBase *QLottiePrecomposition::clone() const
{
    return new QLottiePrecomposition(*this);
}

QLottiePrecomposition *QLottiePrecomposition::construct(const QJsonObject &definition,
                                                        const QMap<QString, QJsonObject> &assets)
{
    std::unique_ptr<QLottiePrecomposition> precomp = std::make_unique<QLottiePrecomposition>();
    if (precomp->parse(definition) < 0)
        return nullptr;

    QJsonArray jsonLayers = definition.value("layers"_L1).toArray();
    if (QLottieLayer::constructLayers(jsonLayers, precomp.get(), assets) < 0)
        return nullptr;

    return precomp.release();
}

int QLottiePrecomposition::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    m_refId = definition.value("id"_L1).toString();

    if (m_refId.isEmpty() || !definition.contains("layers"_L1))
        return -1;

    qCDebug(lcLottieQtLottieParser) << "QLottiePrecomposition::parse():" << m_refId;

    return 0;
}

void QLottiePrecomposition::render(QLottieRenderer &renderer) const
{
    renderer.saveState();
    renderer.render(*this);
    if (!isStructureDumping())
        renderChildren(renderer);
    renderer.restoreState();
}

QT_END_NAMESPACE
