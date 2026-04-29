// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QLOTTIETEXTLAYER_P_H
#define QLOTTIETEXTLAYER_P_H

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

#include <QtGui/QColor>
#include <QtGui/QPainterPath>
#include <QtCore/QMap>
#include <QtCore/QJsonObject>

#include <QtLottie/private/qlottielayer_p.h>

QT_BEGIN_NAMESPACE

class QLottieRenderer;

class Q_LOTTIE_EXPORT QLottieTextLayer : public QLottieLayer
{
public:
    struct TextDocument {
        QString text;
        QString fontFamily;
        qreal fontSize = 12.0;
        QColor fillColor = Qt::black;
        int justification = 0;  // 0=left, 1=right, 2=center
        qreal tracking = 0.0;   // in 1/1000 em units
    };

    QLottieTextLayer() = default;
    explicit QLottieTextLayer(const QLottieTextLayer &other);

    QLottieBase *clone() const override;

    void render(QLottieRenderer &renderer) const override;
    int parse(const QJsonObject &definition) override;

    const TextDocument &textDocument() const { return m_textDocument; }

    // chars: key = ch + "|" + fFamily, value = raw char JSON object from root "chars" array
    void setCharData(const QMap<QString, QJsonObject> &chars);
    QList<QPainterPath> buildGlyphPaths() const;

private:
    struct CharInfo {
        qreal advanceWidth = 0.0;  // in 100-unit em space
        QPainterPath glyphPath;    // in 100-unit em space
    };

    TextDocument m_textDocument;
    QMap<QString, CharInfo> m_chars;
};

QT_END_NAMESPACE

#endif // QLOTTIETEXTLAYER_P_H
