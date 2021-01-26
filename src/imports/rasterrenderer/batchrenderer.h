/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the lottie-qt module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef BATCHRENDERER_H
#define BATCHRENDERER_H

#include <QHash>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

QT_BEGIN_NAMESPACE

class BMBase;
class QImage;
class LottieAnimation;

class BatchRenderer : public QThread
{
    Q_OBJECT

    struct Entry
    {
        LottieAnimation* animator = nullptr;
        BMBase *bmTreeBlueprint = nullptr;
        int startFrame = 0;
        int endFrame = 0;
        int currentFrame = 0;
        int animDir = 1;
        QHash<int, BMBase*> frameCache;
    };

public:
    ~BatchRenderer() override;

    BatchRenderer(BatchRenderer const &) = delete;
    void operator=(BatchRenderer const&) = delete;

    static BatchRenderer *instance();
    static void deleteInstance();

    BMBase *getFrame(LottieAnimation *animator, int frameNumber);

signals:
    void frameReady(LottieAnimation *animator, int frameNumber);

public slots:
    void registerAnimator(LottieAnimation *animator);
    void deregisterAnimator(LottieAnimation *animator);

    bool gotoFrame(LottieAnimation *animator, int frame);

    void frameRendered(LottieAnimation *animator, int frameNumber);

protected:
    void run() override;

    int parse(BMBase *rootElement, const QByteArray &jsonSource) const;

    void prerender(Entry *animEntry);

private:
    BatchRenderer();

    void pruneFrameCache(Entry* e);

private:
    static BatchRenderer *m_rendererInstance;

    QMutex m_mutex;
    QWaitCondition m_waitCondition;

    int m_cacheSize = 2;
    QHash<LottieAnimation *, Entry *> m_animData;
};

QT_END_NAMESPACE

#endif // BATCHRENDERER_H
