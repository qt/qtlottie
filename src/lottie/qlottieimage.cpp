// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottieimage_p.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QString>

#include <QtLottie/private/qlottieconstants_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottieImage::QLottieImage(const QLottieImage &other)
    : QLottieBase(other)
{
    m_url = other.m_url;
    m_size = other.m_size;
    m_image = other.m_image;
}

QLottieImage::QLottieImage(QLottieBase *parent)
{
    setParent(parent);
}

QLottieBase *QLottieImage::clone() const
{
    return new QLottieImage(*this);
}

int QLottieImage::construct(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottieImage::parse():" << m_name;

    QJsonObject asset = definition.value(u"asset"_s).toObject();
    if (!checkRequiredKey(asset, u"Image"_s, {u"p"_s, u"w"_s, u"h"_s}, m_name))
        return -1;

    QString assetString = asset.value(u"p"_s).toString();

    if (assetString.startsWith(u"data:image"_s)) {
        QStringList assetsDataStringList = assetString.split(u","_s);
        if (assetsDataStringList.size() > 1) {
            QByteArray assetData = QByteArray::fromBase64(assetsDataStringList[1].toLatin1());
            m_image.loadFromData(assetData);
        }
        if (m_image.isNull()) {
            qCWarning(lcLottieQtLottieParser) << "Unable to load embedded image asset"
                                              << asset.value(u"id"_s).toString();
        }
    } else {
        QFileInfo info(asset.value(u"fileSource"_s).toString());
        QString urlPath = info.path() + QLatin1Char('/')
                + asset.value(u"u"_s).toString() + QLatin1Char('/') + assetString;
        m_url = QUrl(urlPath);
        m_url.setScheme(u"file"_s);
        QString path = m_url.toLocalFile();
        m_image.load(path);
        if (m_image.isNull())
            qCWarning(lcLottieQtLottieParser) << "Unable to load file" << path;
    }

    const qreal width = asset.value(u"w"_s).toDouble();
    const qreal height = asset.value(u"h"_s).toDouble();
    m_size = QSizeF(width, height);

    return 0;
}

void QLottieImage::render(QLottieRenderer &renderer) const
{
    renderer.render(*this);
}

QT_END_NAMESPACE
