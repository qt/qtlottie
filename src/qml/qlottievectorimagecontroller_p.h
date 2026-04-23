// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QLOTTIEVECTORIMAGECONTROLLER_P_H
#define QLOTTIEVECTORIMAGECONTROLLER_P_H

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

#include <QtCore/qobject.h>
#include <QtLottie/qtlottieexports.h>
#include <QtQuickVectorImage/private/qquickvectorimage_p.h>

QT_BEGIN_NAMESPACE

class Q_LOTTIE_EXPORT QLottieVectorImageController : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LottieVectorImageController)
    QML_ADDED_IN_VERSION(6, 12)

    Q_PROPERTY(QQuickVectorImage *target READ target WRITE setTarget NOTIFY targetChanged FINAL)
    Q_PROPERTY(qreal frameRate READ frameRate WRITE setFrameRate RESET resetFrameRate NOTIFY frameRateChanged)
    Q_PROPERTY(qreal startFrame READ startFrame NOTIFY startFrameChanged)
    Q_PROPERTY(qreal endFrame READ endFrame NOTIFY endFrameChanged)

public:
    QLottieVectorImageController(QObject *parent = nullptr);

    QQuickVectorImage *target() const;
    void setTarget(QQuickVectorImage *newTarget);

    qreal frameRate() const;
    void setFrameRate(qreal newFrameRate);
    void resetFrameRate();

    qreal startFrame() const;
    qreal endFrame() const;

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void gotoAndPlay(qreal frame);
    Q_INVOKABLE void gotoAndStop(qreal frame);

signals:
    void targetChanged();
    void frameRateChanged();
    void startFrameChanged();
    void endFrameChanged();

private:
    void gotoFrame(qreal frame);
    QQuickVectorImage *m_target = nullptr;
    qreal m_animFrameRate = 30;
};

QT_END_NAMESPACE

#endif // QLOTTIEVECTORIMAGECONTROLLER_P_H

