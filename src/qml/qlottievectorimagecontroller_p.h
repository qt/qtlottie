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
#include <QtCore/qpointer.h>
#include <QtQml/qqmlparserstatus.h>
#include <QtLottie/qtlottieexports.h>
#include <QtQuickVectorImage/private/qquickvectorimage_p.h>

QT_BEGIN_NAMESPACE

class Q_LOTTIE_EXPORT QLottieVectorImageController : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_NAMED_ELEMENT(LottieVectorImageController)
    QML_ADDED_IN_VERSION(6, 12)

    Q_PROPERTY(QQuickVectorImage *target READ target WRITE setTarget NOTIFY targetChanged FINAL)
    Q_PROPERTY(qreal frameRate READ frameRate WRITE setFrameRate RESET resetFrameRate NOTIFY frameRateChanged FINAL)
    Q_PROPERTY(qreal startFrame READ startFrame NOTIFY startFrameChanged FINAL)
    Q_PROPERTY(qreal endFrame READ endFrame NOTIFY endFrameChanged FINAL)
    Q_PROPERTY(qreal currentFrame READ currentFrame NOTIFY currentFrameChanged FINAL)

public:
    QLottieVectorImageController(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QQuickVectorImage *target() const;
    void setTarget(QQuickVectorImage *newTarget);

    qreal frameRate() const;
    void setFrameRate(qreal newFrameRate);
    void resetFrameRate();

    qreal startFrame() const;
    qreal endFrame() const;
    qreal currentFrame() const;

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
    void currentFrameChanged();

private:
    void gotoFrame(qreal frame);
    void onGeneratedItemChanged();
    QPointer<QQuickVectorImage> m_target;
    QPointer<QQuickItem> m_generatedItem;
    qreal m_animFrameRate = 30;
};

QT_END_NAMESPACE

#endif // QLOTTIEVECTORIMAGECONTROLLER_P_H

