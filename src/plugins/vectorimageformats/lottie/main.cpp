// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtQuickVectorImageGenerator/private/qquickvectorimageplugin_p.h>
#include <QtLottieVectorImageGenerator/private/qlottievisitor_p.h>
#include <QtLottie/private/qlottieroot_p.h>
#include <QtLottie/private/qlottieprecomposition_p.h>
#include <QtCore/qfile.h>
#include <QtCore/qbuffer.h>
#include <QtCore/qscopeguard.h>

#include <QtQuick/private/qquickanimation_p.h>

QT_BEGIN_NAMESPACE

class QLottieVectorImagePluginGenerator : public QQuickVectorImagePluginGenerator
{
public:
    bool generate(QQuickGenerator *generator) override;
};

bool QLottieVectorImagePluginGenerator::generate(QQuickGenerator *generator)
{
    QLottieRoot root;

    const QQuickVectorImageSource source = generator->source();
    QByteArray jsonSource = source.data();
    if (jsonSource.isEmpty()) {
        QFile f(source.fileName());
        if (f.open(QIODevice::ReadOnly))
            jsonSource = f.readAll();
    }

    if (!jsonSource.isEmpty()) {
        if (root.parseSource(jsonSource, QUrl::fromLocalFile(source.fileName())) >= 0) {
            root.setStructureDumping(true);
            root.updateProperties(0);

            if (std::optional<qint64> envFrame = qEnvironmentVariableIntegerValue("QLT_FRAMENO")) {
                qint64 freezeFrame = *envFrame >= 0 ? *envFrame : (root.endFrame() - root.startFrame()) / 2;
                qint64 freezeTime = 1000 * freezeFrame / root.frameRate();
                qputenv("QT_QUICKVECTORIMAGE_FREEZE", QByteArray::number(freezeTime));
            }

            generator->addExtraImport(QStringLiteral("Qt.labs.lottieqt.VectorImageHelpers"));
            generator->setGeneratorFlags(
                generator->generatorFlags().setFlag(QQuickVectorImageGenerator::TimelineAnimation));
            QLottieVisitor visitor(generator);
            visitor.render(root);

            return true;
        }
    }

    return false;
}

class QLottieVectorImagePlugin : public QObject, public QQuickVectorImagePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQuickVectorImageFormatsPluginFactory_iid FILE "lottie.json")
    Q_INTERFACES(QQuickVectorImagePlugin)
public:
    QLottieVectorImagePlugin();
    ~QLottieVectorImagePlugin();

    QQuickVectorImagePluginGenerator *createGenerator(const QQuickVectorImageSource &source) override;

private:
    bool canRead(QIODevice *input) const;
};

QLottieVectorImagePlugin::QLottieVectorImagePlugin()
{
}

QLottieVectorImagePlugin::~QLottieVectorImagePlugin()
{
}

QQuickVectorImagePluginGenerator *QLottieVectorImagePlugin::createGenerator(const QQuickVectorImageSource &source)
{
    std::unique_ptr<QIODevice> device;

    QByteArray ba = source.data();
    if (source.isFile())
        device.reset(new QFile(source.fileName()));
    else
        device.reset(new QBuffer(&ba));

    if (!device->open(QIODevice::ReadOnly))
        return nullptr;

    if (!canRead(device.get()))
        return nullptr;

    return new QLottieVectorImagePluginGenerator;
}

bool QLottieVectorImagePlugin::canRead(QIODevice *input) const
{
    const qint64 pos = input->pos();
    auto cleanup = qScopeGuard([&] { input->seek(pos); });
    QTextStream s(input);
    const QString head = s.read(256);
    bool res = QStringView(head).trimmed().startsWith(QChar::fromLatin1('{'));
    return res;
}

QT_END_NAMESPACE

#include "main.moc"
