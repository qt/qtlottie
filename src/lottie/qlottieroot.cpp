// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottieroot_p.h"
#include "qlottielayer_p.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QString>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottieRoot::QLottieRoot(const QLottieRoot &other)
    : QLottieBase(other)
{
}

QLottieBase *QLottieRoot::clone() const
{
    return new QLottieRoot(*this);
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
    QJsonArray jsonLayers = rootObj.value(u"layers"_s).toArray();
    QJsonArray jsonAssets = rootObj.value(u"assets"_s).toArray();
    QString name = rootObj.value(u"nm"_s).toString();

    if (!checkRequiredKey(rootObj, u""_s, {u"fr"_s, u"ip"_s, u"op"_s}, name))
        return -1;

    m_frameRate = rootObj.value(u"fr"_s).toVariant().toInt();
    if (m_frameRate <= 0) {
        qCWarning(lcLottieQtLottieParser) << "\"fr\" value of" << name << "should be greater than 0";
        return -1;
    }

    m_startFrame = rootObj.value(u"ip"_s).toVariant().toInt();
    m_endFrame = rootObj.value(u"op"_s).toVariant().toInt();

    m_size = QSize(rootObj.value(u"w"_s).toInt(-1), rootObj.value(u"h"_s).toInt(-1));

    QJsonArray::const_iterator jsonAssetsIt = jsonAssets.constBegin();
    while (jsonAssetsIt != jsonAssets.constEnd()) {
        QJsonObject jsonAsset = (*jsonAssetsIt).toObject();

        jsonAsset.insert(u"fileSource"_s, QJsonValue::fromVariant(fileSource));
        if (!checkRequiredKey(jsonAsset, u"Asset"_s, {u"id"_s}))
            return -1;

        QString id = jsonAsset.value(u"id"_s).toString();
        assets.insert(id, jsonAsset);
        jsonAssetsIt++;
    }

    int ret = QLottieLayer::constructLayers(jsonLayers, this, assets);

    return ret;
}

void QLottieRoot::setStructureDumping(bool enabled)
{
    m_structureDumping = enabled ? 1 : 0;
}

QT_END_NAMESPACE
