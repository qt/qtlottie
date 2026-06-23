// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qlottievectorimagecontroller_p.h"
#include <QtQml/private/qqmlbuiltins_p.h>
#include <QtQml/qqmlproperty.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/private/qquickanimation_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype LottieVectorImageController
    \inqmlmodule Qt.labs.lottieqt
    \inherits QtObject
    \since 6.12
    \brief Provides animation control for a VectorImage that is displaying a Lottie file.

    VectorImage can display and play Lottie animations, but by itself it provides only rudimentary
    functionality for controlling the playback. LottieVectorImageController extends VectorImage
    with more powerful API for querying and controlling the animation.

    LottieVectorImageController's \l target property specifies the VectorImage to be controlled. It
    is set by default if the LottieVectorImageController is created as a child or property of a
    VectorImage. When the VectorImage has loaded a Lottie file, and its \l {VectorImage::status}
    {status} is \c Ready, the LottieVectorImageController's properties can be read and written, and
    its functions called, to control the playback.

    Taken together with VectorImage, LottieVectorImageController mimics the API of the existing
    LottieAnimation item, so they are typically interchangeable with little effort. (The \c
    getDuration() function is not provided, since it can be trivially replaced by a simple
    expression using the endFrame, startFrame and optionally frameRate properties.)

    \sa VectorImage, {qtlottieviewer Example}
*/

QLottieVectorImageController::QLottieVectorImageController(QObject *parent)
    : QObject(parent), QQmlParserStatus()
{
}

/*!
    \internal
*/
void QLottieVectorImageController::classBegin()
{
}

/*!
    \internal
*/
void QLottieVectorImageController::componentComplete()
{
    if (!m_target) {
        if (auto *p = qobject_cast<QQuickVectorImage *>(parent()))
            setTarget(p);
    }
}

/*!
    \qmlproperty VectorImage LottieVectorImageController::target

    This property specifies the VectorImage to be controlled.

    If the LottieVectorImageController is created as a child or property of a VectorImage, that
    will be the default target. Otherwise, the target must be set explicitly.
*/

QQuickVectorImage *QLottieVectorImageController::target() const
{
    return m_target;
}

void QLottieVectorImageController::setTarget(QQuickVectorImage *newTarget)
{
    if (m_target == newTarget)
        return;

    if (m_target)
        disconnect(m_target, &QQuickVectorImage::generatedItemChanged, this, &QLottieVectorImageController::onGeneratedItemChanged);

    m_target = newTarget;

    if (m_target)
        connect(m_target, &QQuickVectorImage::generatedItemChanged, this, &QLottieVectorImageController::onGeneratedItemChanged);

    onGeneratedItemChanged();

    emit targetChanged();
}

void QLottieVectorImageController::onGeneratedItemChanged()
{
    if (m_generatedItem)
        QObject::disconnect(m_generatedItem, nullptr, this, nullptr);

    const qreal oldStartFrame = startFrame();
    const qreal oldEndFrame = endFrame();
    const qreal oldFrameRate = frameRate();
    const qreal oldCurrentFrame = currentFrame();

    m_generatedItem = m_target ? m_target->generatedItem() : nullptr;
    m_animFrameRate = frameRate();

    if (m_generatedItem) {
        const QQmlProperty prop(m_generatedItem, QStringLiteral("frameCounter"));
        if (prop.isValid())
            prop.connectNotifySignal(this, SIGNAL(currentFrameChanged()));
    }

    if (startFrame() != oldStartFrame)
        emit startFrameChanged();
    if (endFrame() != oldEndFrame)
        emit endFrameChanged();
    if (m_animFrameRate != oldFrameRate)
        emit frameRateChanged();
    if (currentFrame() != oldCurrentFrame)
        emit currentFrameChanged();
}

/*!
    \qmlproperty real LottieVectorImageController::frameRate

    This property holds the animation playback speed in frames per second.

    When the vector image is loaded, this property is set to the frame rate value specified in the
    source file.

    After loading, frameRate can be modified to change the playback speed. Assigning an empty
    value to it will reset it to the source file value.
*/

qreal QLottieVectorImageController::frameRate() const
{
    return m_generatedItem ? m_generatedItem->property("frameRate").toReal() : 30;
}

void QLottieVectorImageController::setFrameRate(qreal newFrameRate)
{
    QQuickItem *item = m_generatedItem;
    if (item && item->property("frameRate").toReal() != newFrameRate) {
        const qreal currentFrame = item->property("frameCounter").toReal();
        item->setProperty("frameRate", QVariant(newFrameRate));
        auto *anim = item->findChild<QQuickNumberAnimation *>("_qt_frameCounterAnimation", Qt::FindDirectChildrenOnly);
        if (anim && m_target) {
            // Force the NumberAnimation to change speed now, then restore pause state & frame
            anim->restart();
            anim->setPaused(m_target->animations()->paused());
            gotoFrame(currentFrame);
        }
        emit frameRateChanged();
    }
}

void QLottieVectorImageController::resetFrameRate()
{
    setFrameRate(m_animFrameRate);
}

/*!
    \qmlproperty real LottieVectorImageController::startFrame
    \readonly

    The animation's starting frame number ("in-frame"), as specified in the source file.
*/
qreal QLottieVectorImageController::startFrame() const
{
    return m_generatedItem ? m_generatedItem->property("startFrame").toReal() : 0;
}

/*!
    \qmlproperty real LottieVectorImageController::endFrame
    \readonly

    The animation's ending frame number ("out-frame"), as specified in the source file.
*/
qreal QLottieVectorImageController::endFrame() const
{
    return m_generatedItem ? m_generatedItem->property("endFrame").toReal() : 0;
}

/*!
    \qmlproperty real LottieVectorImageController::currentFrame
    \readonly

    The number of the currently displayed animation frame.
*/
qreal QLottieVectorImageController::currentFrame() const
{
    return m_generatedItem ? m_generatedItem->property("frameCounter").toReal() : 0;
}

/*!
    \qmlmethod void LottieVectorImageController::play()

    Starts (un-pauses) the playback.
*/
void QLottieVectorImageController::play()
{
    if (m_target)
        m_target->animations()->setPaused(false);
}

/*!
    \qmlmethod void LottieVectorImageController::pause()

    Pauses the playback.
*/
void QLottieVectorImageController::pause()
{
    if (m_target)
        m_target->animations()->setPaused(true);
}

/*!
    \qmlmethod void LottieVectorImageController::togglePause()

    Toggles the \l target VectorImage between playing and paused modes.
*/
void QLottieVectorImageController::togglePause()
{
    if (m_target) {
        const bool isPaused = m_target->animations()->paused();
        m_target->animations()->setPaused(!isPaused);
    }
}

/*!
    \qmlmethod void LottieVectorImageController::start()

    Restarts the playback from the startFrame.
*/
void QLottieVectorImageController::start()
{
    gotoAndPlay(startFrame());
}

/*!
    \qmlmethod void LottieVectorImageController::stop()

    Resets the playback to the startFrame and pauses it.
*/
void QLottieVectorImageController::stop()
{
    gotoAndStop(startFrame());
}

/*!
    \qmlmethod void LottieVectorImageController::gotoAndPlay(real frame)

    Moves the playback to Lottie frame number \a frame and plays it.

    \a frame will be clamped between startFrame and endFrame.
*/
void QLottieVectorImageController::gotoAndPlay(qreal frame)
{
    gotoFrame(frame);
    play();
}

/*!
    \qmlmethod void LottieVectorImageController::gotoAndStop(real frame)

    Moves the playback to Lottie frame number \a frame and pauses it.

    \a frame will be clamped between startFrame and endFrame.
*/
void QLottieVectorImageController::gotoAndStop(qreal frame)
{
    pause();
    gotoFrame(frame);
}

void QLottieVectorImageController::gotoFrame(qreal frame)
{
    if (m_generatedItem) {
        auto *anim = m_generatedItem->findChild<QQuickNumberAnimation *>("_qt_frameCounterAnimation",
                                                                         Qt::FindDirectChildrenOnly);
        if (anim) {
            const qreal from = anim->from();
            const qreal to = anim->to();
            const qreal rawProgress = (frame - from) / (to - from);
            if (qIsFinite(rawProgress)) {
                const qreal progress = qBound(qreal(0), rawProgress, qreal(1));
                anim->setCurrentTime(qRound(progress * anim->duration()));
            }
        }
    }
}

QT_END_NAMESPACE
