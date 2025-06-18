// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQuickVectorImageGenerator/private/qquickvectorimageplugin_p.h>
#include <QtLottieVectorImageGenerator/private/qlottievisitor_p.h>
#include <QtLottie/private/qlottieroot_p.h>
#include <QtCore/qfile.h>

QT_BEGIN_NAMESPACE

class QLottieVectorImagePlugin : public QObject, public QQuickVectorImagePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQuickVectorImageFormatsPluginFactory_iid FILE "lottie.json")
    Q_INTERFACES(QQuickVectorImagePlugin)
public:
    QLottieVectorImagePlugin();
    ~QLottieVectorImagePlugin();

    bool generate(const QString &fileName, QQuickItemGenerator *generator) override;
};

QLottieVectorImagePlugin::QLottieVectorImagePlugin()
{
}

QLottieVectorImagePlugin::~QLottieVectorImagePlugin()
{
}

bool QLottieVectorImagePlugin::generate(const QString &fileName, QQuickItemGenerator *generator)
{
    QFile f(fileName);
    QLottieRoot root;

    if (f.open(QIODevice::ReadOnly)) {
        QByteArray jsonSource = f.readAll();

        static int frameNo = qEnvironmentVariableIntValue("QLT_FRAMENO");

        if (!root.parseSource(jsonSource, fileName)) {
            if (frameNo < 0)
                frameNo = (root.endFrame() - root.startFrame()) / 2;

            root.setStructureDumping(true);
            for (QLottieBase *elem : root.children()) {
                if (elem->active(frameNo))
                    elem->updateProperties(frameNo);
            }

            generator->addExtraImport(QLatin1String("Qt.labs.lottieqt.VectorImageHelpers"));
            QLottieVisitor visitor(fileName, generator);
            visitor.render(root);
            return true;
        }
    }

    return false;
}

QT_END_NAMESPACE

#include "main.moc"
