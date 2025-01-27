// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottieroot_p.h"
#include "qlottielayer_p.h"

#include <QJsonDocument>
#include <QJsonArray>

QT_BEGIN_NAMESPACE

QLottieRoot::QLottieRoot(const QLottieRoot &other)
    : QLottieBase(other)
{
}

QLottieBase *QLottieRoot::clone() const
{
    return new QLottieRoot(*this);
}

int QLottieRoot::parseSource(const QByteArray &jsonSource, const QUrl &fileSource, QVersionNumber version)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonSource);
    QJsonObject rootObj = doc.object();

    if (rootObj.empty())
        return -1;

    if (version.isNull())
        version = QVersionNumber::fromString(rootObj.value(QLatin1String("v")).toString());

    QMap<QString, QJsonObject> assets;
    QJsonArray jsonLayers = rootObj.value(QLatin1String("layers")).toArray();
    QJsonArray jsonAssets = rootObj.value(QLatin1String("assets")).toArray();
    QJsonArray::const_iterator jsonAssetsIt = jsonAssets.constBegin();
    while (jsonAssetsIt != jsonAssets.constEnd()) {
        QJsonObject jsonAsset = (*jsonAssetsIt).toObject();

        jsonAsset.insert(QLatin1String("fileSource"), QJsonValue::fromVariant(fileSource));
        QString id = jsonAsset.value(QLatin1String("id")).toString();
        assets.insert(id, jsonAsset);
        jsonAssetsIt++;
    }

    QJsonArray::const_iterator jsonLayerIt = jsonLayers.constEnd();
    while (jsonLayerIt != jsonLayers.constBegin()) {
        jsonLayerIt--;
        QJsonObject jsonLayer = (*jsonLayerIt).toObject();
        if (jsonLayer.value(QLatin1String("ty")).toInt() == 2) {
            QString refId = jsonLayer.value(QLatin1String("refId")).toString();
            jsonLayer.insert(QLatin1String("asset"), assets.value(refId));
        }
        QLottieLayer *layer = QLottieLayer::construct(jsonLayer, version);
        if (layer) {
            layer->setParent(this);
            // Mask layers must be rendered before the layers they affect to
            // although they appear after in layer hierarchy. For this reason
            // move a mask in front of the affected layer, so it will be rendered first
            if (layer->isMaskLayer())
                insertChildBeforeLast(layer);
            else
                appendChild(layer);
        }
    }

    return 0;
}

QT_END_NAMESPACE
