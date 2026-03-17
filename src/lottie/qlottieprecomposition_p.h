// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QLOTTIEPRECOMPOSITION_P_H
#define QLOTTIEPRECOMPOSITION_P_H

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

#include <QtLottie/private/qlottiebase_p.h>

QT_BEGIN_NAMESPACE

class Q_LOTTIE_EXPORT QLottiePrecomposition : public QLottieBase
{
public:
    QLottiePrecomposition();
    explicit QLottiePrecomposition(const QLottiePrecomposition &other);
    QLottieBase *clone() const override;

    static QLottiePrecomposition *construct(const QJsonObject &definition,
                                            const QMap<QString, QJsonObject> &assets);
    int parse(const QJsonObject &definition) override;
    void render(QLottieRenderer &renderer) const override;

    QString refId() const { return m_refId; }

protected:
    QString m_refId;
};

QT_END_NAMESPACE

#endif // QLOTTIEPRECOMPOSITION_P_H
