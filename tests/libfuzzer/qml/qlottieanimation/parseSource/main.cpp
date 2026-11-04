// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QUrl>
#include <QtLottie/private/qlottieroot_p.h>

static QtMessageHandler mh =
        qInstallMessageHandler([](QtMsgType, const QMessageLogContext &, const QString &) {});

extern "C" int LLVMFuzzerTestOneInput(const char *Data, size_t Size)
{
    static int argc = 3;
    static char arg1[] = "fuzzer";
    static char arg2[] = "-platform";
    static char arg3[] = "minimal";
    static char *argv[] = { arg1, arg2, arg3, nullptr };
    static QGuiApplication qga(argc, argv);
    QLottieRoot root;
    root.parseSource(QByteArray::fromRawData(Data, Size), QUrl());
    return 0;
}
