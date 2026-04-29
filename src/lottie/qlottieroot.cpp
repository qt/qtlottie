// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qlottieroot_p.h"
#include "qlottielayer_p.h"
#include "qlottieprecomplayer_p.h"
#include "qlottieprecomposition_p.h"
#include "qlottietextlayer_p.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QString>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

namespace {

void distributeChars(QLottieBase *container, const QMap<QString, QJsonObject> &chars)
{
    for (QLottieBase *child : container->children()) {
        if (child->type() == LOTTIE_LAYER_TEXT_IX)
            static_cast<QLottieTextLayer *>(child)->setCharData(chars);
    }
}

} // namespace

QLottieRoot::QLottieRoot()
{
    m_type = LOTTIE_ROOT_IX;
}

QLottieRoot::QLottieRoot(const QLottieRoot &other)
    : QLottieBase(other)
{
    m_size = other.m_size;
    m_frameRate = other.m_frameRate;
    m_startFrame = other.m_startFrame;
    m_endFrame = other.m_endFrame;
}

QLottieRoot::~QLottieRoot()
{
    qDeleteAll(m_precompositions);
    m_precompositions.clear();
}

QLottieBase *QLottieRoot::clone() const
{
    return new QLottieRoot(*this);
}

void QLottieRoot::updateProperties(int frame)
{
    for (QLottiePrecomposition *precomp : m_precompositions)
        precomp->updateProperties(frame);

    QLottieBase::updateProperties(frame);
}

QSize QLottieRoot::layerSize() const
{
    return m_size;
}

int QLottieRoot::parseSource(const QByteArray &jsonSource, const QUrl &fileSource)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonSource);
    QJsonObject rootObj = doc.object();
    m_definition = rootObj;

    if (rootObj.empty())
        return -1;

    QMap<QString, QJsonObject> assets;
    QJsonArray jsonLayers = rootObj.value("layers"_L1).toArray();
    QJsonArray jsonAssets = rootObj.value("assets"_L1).toArray();
    QString name = rootObj.value("nm"_L1).toString();

    if (!checkRequiredKeys(rootObj, ""_L1, { "fr"_L1, "ip"_L1, "op"_L1 }, name))
        return -1;

    m_frameRate = rootObj.value("fr"_L1).toVariant().toInt();
    if (m_frameRate <= 0) {
        qCWarning(lcLottieQtLottieParser) << "\"fr\" value of" << name << "should be greater than 0";
        return -1;
    }

    m_startFrame = rootObj.value("ip"_L1).toVariant().toInt();
    m_endFrame = rootObj.value("op"_L1).toVariant().toInt();
    if (m_endFrame <= m_startFrame) {
        qCWarning(lcLottieQtLottieParser) << "Invalid top level  \"ip\"/\"op\" values";
        return -1;
    }

    m_size = QSize(rootObj.value("w"_L1).toInt(-1), rootObj.value("h"_L1).toInt(-1));

    QJsonArray::const_iterator jsonAssetsIt = jsonAssets.constBegin();
    while (jsonAssetsIt != jsonAssets.constEnd()) {
        QJsonObject jsonAsset = (*jsonAssetsIt).toObject();

        jsonAsset.insert("fileSource"_L1, QJsonValue::fromVariant(fileSource));
        if (!checkRequiredKeys(jsonAsset, "Asset"_L1, { "id"_L1 }))
            return -1;

        QString id = jsonAsset.value("id"_L1).toString();
        assets.insert(id, jsonAsset);
        jsonAssetsIt++;
    }

    QMap<QString, QJsonObject> chars;
    const QJsonArray jsonChars = rootObj.value("chars"_L1).toArray();
    if (!jsonChars.isEmpty()) {
        // "chars" entries identify fonts by (fFamily, style), but text layers
        // reference fonts by fName. fName is not always "fFamily-fStyle"
        // e.g. regular Helvetica has fName="Helvetica", not "Helvetica-Regular"
        // while light Helvetica has fName="Helvetica-Light" (fFamily-fStyle).
        // so we build a lookup from (fFamily, style) to fName via fonts.list.
        QMap<QPair<QString, QString>, QString> fontNameMap;
        const QJsonArray fontList = rootObj.value("fonts"_L1).toObject().value("list"_L1).toArray();
        for (const QJsonValue &fontVal : fontList) {
            const QJsonObject fontObj = fontVal.toObject();
            fontNameMap.insert({fontObj.value("fFamily"_L1).toString(),
                                 fontObj.value("fStyle"_L1).toString()},
                                fontObj.value("fName"_L1).toString());
        }
        for (const QJsonValue &charVal : jsonChars) {
            const QJsonObject charObj = charVal.toObject();
            const QString ch = charObj.value("ch"_L1).toString();
            const QString fFamily = charObj.value("fFamily"_L1).toString();
            const QString style = charObj.value("style"_L1).toString();
            const QString fName = fontNameMap.value({fFamily, style}, fFamily);
            if (!ch.isEmpty())
                chars.insert(ch + u'|' + fName, charObj);
        }
    }

    for (const auto &[id, jsonAsset] : assets.asKeyValueRange()) {
        auto *precomp = QLottiePrecomposition::construct(jsonAsset, assets);
        if (precomp) {
            precomp->setParent(this);
            if (!chars.isEmpty())
                distributeChars(precomp, chars);
            m_precompositions.insert(id, precomp);
        }
    }

    int ret = QLottieLayer::constructLayers(jsonLayers, this, assets);
    if (!chars.isEmpty())
        distributeChars(this, chars);

    // Resolve precomp layers
    QStack<QString> visitedIds;
    for (QLottieBase *element : m_precompositions) {
        if (!resolvePrecompLayers(element, &visitedIds)) {
            qCWarning(lcLottieQtLottieParser) << "Precomp layer resolving failed [assets]";
            return -1;
        }
    }
    if (!resolvePrecompLayers(this, &visitedIds)) {
        qCWarning(lcLottieQtLottieParser) << "Precomp layer resolving failed [root]";
        return -1;
    }

    return ret;
}

void QLottieRoot::setStructureDumping(bool enabled)
{
    m_structureDumping = enabled ? 1 : 0;
}

bool QLottieRoot::resolvePrecompLayers(QLottieBase *element, QStack<QString> *visitedIds)
{
    QLottiePrecompLayer *precompLayer = nullptr;
    const int elemType = element->type();
    if (elemType == LOTTIE_LAYER_PRECOMP_IX) {
        precompLayer = static_cast<QLottiePrecompLayer *>(element);
        const QString refId = precompLayer->refId();
        if (visitedIds->contains(refId)) {
            qCWarning(lcLottieQtLottieParser) << "Cyclical precomp reference detected";
            return false;
        }
        QLottiePrecomposition *precomp = m_precompositions.value(refId);
        if (!precomp) {
            qCWarning(lcLottieQtLottieParser) << "Precomp reference not found:" << refId;
            return false;
        }
        visitedIds->push(refId);
        if (precompLayer->children().size() == 0) { // Not already resolved
            QLottieBase *clone = precomp->clone();
            clone->setParent(precompLayer);
            precompLayer->appendChild(clone);
        }
    }

    if (precompLayer || elemType == LOTTIE_ROOT_IX || elemType == LOTTIE_PRECOMPOSITION_IX) {
        for (QLottieBase *child : element->children()) {
            if (!resolvePrecompLayers(child, visitedIds))
                return false;
        }
    }

    if (precompLayer)
        visitedIds->pop();

    return true;
}

QT_END_NAMESPACE
