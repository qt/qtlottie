// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#ifndef QLOTTIEPATHTRIMMER_P_H
#define QLOTTIEPATHTRIMMER_P_H

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

#include <QList>

#include <QtLottie/qlottieglobal.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QJsonObject;
class QLottieQTrimPath;
class QLottieRenderer;
class QLottieBase;
class QLottieShape;

class LOTTIE_EXPORT QLottiePathTrimmer
{
public:
    QLottiePathTrimmer(QLottieBase *root);

    void addTrim(QLottieQTrimPath* trim);
    bool inUse() const;

    void applyTrim(QLottieShape *shape);

    void updateProperties(int frame);
    void render(QLottieRenderer &renderer) const;

private:
    QLottieBase *m_root = nullptr;

    QList<QLottieQTrimPath*> m_trimPaths;
    QLottieQTrimPath *m_appliedTrim = nullptr;
};

QT_END_NAMESPACE

#endif // QLOTTIEPATHTRIMMER_P_H

