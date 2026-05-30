// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QEventLoop>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QTimer>
#include <QtGlobal>
#include <QtLottie/private/qlottieanimation_p.h>

class QLottieAnimationFuzzer : public QLottieAnimation
{
    Q_OBJECT
public:
    explicit QLottieAnimationFuzzer(QQuickItem *parent = nullptr)
        : QLottieAnimation(parent)
    {
    }

    void fuzzRender(const QByteArray &jsonSource)
    {
        // Go through the full pipeline: parse + batch renderer registration
        loadFinished(jsonSource);

        if (m_status != QLottieAnimation::Ready)
            return;

        // Wait for the batch renderer to produce the first frame.
        // Poll via renderNextFrame() which calls getFrame() internally;
        // once it returns without setting m_waitForFrameConn the frame
        // is in the cache and paint() will draw something.
        QDeadlineTimer deadline(500);
        while (!deadline.hasExpired()) {
            renderNextFrame();
            if (!m_waitForFrameConn)
                break;
            QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        }

        // Exercise the rendering pipeline with a bounded offscreen image
        const int w = qBound(1, qRound(m_animWidth), 4096);
        const int h = qBound(1, qRound(m_animHeight), 4096);
        QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&image);
        paint(&painter);
    }
};

// silence warnings
static QtMessageHandler mh = qInstallMessageHandler([](QtMsgType, const QMessageLogContext &,
                                                       const QString &) {});

extern "C" int LLVMFuzzerTestOneInput(const char *Data, size_t Size) {
    static int argc = 3;
    static char arg1[] = "fuzzer";
    static char arg2[] = "-platform";
    static char arg3[] = "minimal";
    static char *argv[] = {arg1, arg2, arg3, nullptr};
    static QGuiApplication qga(argc, argv);
    static QLottieAnimationFuzzer lottie;
    lottie.fuzzRender(QByteArray::fromRawData(Data, Size));
    return 0;
}

#include "main.moc"
