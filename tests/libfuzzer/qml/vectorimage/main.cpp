// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QtGlobal>
#include <QTemporaryFile>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QtQuickVectorImage/private/qquickvectorimage_p.h>

// silence warnings
static QtMessageHandler mh = qInstallMessageHandler([](QtMsgType, const QMessageLogContext &,
                                                       const QString &) {});

// Driver may allocate caches or other persistent state, which we need to suppress.
// Add any false positive leaks here.
extern "C" const char *__lsan_default_suppressions()
{
    return "leak:libgallium\n"
           "leak:swrast\n"
           "leak:libGLX\n"
           "leak:libEGL\n"
           "leak:libglapi\n"
           "leak:swiftshader\n"
           "leak:libvulkan\n";
}

extern "C" int LLVMFuzzerInitialize(int *_argc, char ***_argv)
{
    Q_UNUSED(_argc);
    Q_UNUSED(_argv);

    QHashSeed::setDeterministicGlobalSeed();
    static int argc = 1;
    static char arg1[] = "fuzzer";
    static char *argv[] = {arg1, nullptr};

    static QGuiApplication *qga = new QGuiApplication(argc, argv);
    Q_UNUSED(qga);

    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const char *Data, size_t Size)
{
    // ### QQuickVectorImage cannot load from a QByteArray yet, so pass the data via a file
    QTemporaryFile jsonFile;
    if (!jsonFile.open())
        return 0;
    jsonFile.write(Data, qint64(Size));
    if (!jsonFile.flush())
        return 0;

    QQuickWindow window;
    window.resize(512, 512);
    window.create();

    QQmlEngine engine;
    QQuickVectorImage vectorImage;
    QQmlEngine::setContextForObject(&vectorImage, engine.rootContext());
    vectorImage.setParentItem(window.contentItem());
    vectorImage.setSize(window.size());
    vectorImage.setAssumeTrustedSource(true);
    vectorImage.setSource(QUrl::fromLocalFile(jsonFile.fileName()));

    window.grabWindow(); // renders one frame

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    return 0;
}
