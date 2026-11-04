// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>

#include "private/qlottieconstants_p.h"
#include "private/qlottieroot_p.h"

using namespace Qt::StringLiterals;

class tst_QLottieParse : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void maxNestingDepth();
    void maxNestingDepthFromEnvironment_data();
    void maxNestingDepthFromEnvironment();
    void shapeGroups_data();
    void shapeGroups();
    void effectGroups_data();
    void effectGroups();
    void precompChain_data();
    void precompChain();

private:
    static QJsonObject transform();
    static QJsonObject shapeLayer(const QJsonArray &shapes);
    static QJsonObject precompLayer(const QString &refId);
    static QByteArray document(const QJsonArray &layers, const QJsonArray &assets = {});
    static int groupDepth(const QLottieBase *element);
};

void tst_QLottieParse::init()
{
    QTest::failOnWarning();
}

void tst_QLottieParse::maxNestingDepth()
{
    const std::optional<qint64> expected =
            qEnvironmentVariableIntegerValue("TST_QLOTTIEPARSE_EXPECTED_DEPTH");
    if (!expected && qEnvironmentVariableIsSet("QLOTTIE_MAX_NESTING_DEPTH"))
        QSKIP("Running with QLOTTIE_MAX_NESTING_DEPTH set");
    QCOMPARE(qLottieMaxNestingDepth(), int(expected.value_or(64)));
}

void tst_QLottieParse::maxNestingDepthFromEnvironment_data()
{
    QTest::addColumn<QString>("value");
    QTest::addColumn<int>("expected");

    QTest::newRow("smaller") << u"8"_s << 8;
    QTest::newRow("larger") << u"100"_s << 100;
    QTest::newRow("zero") << u"0"_s << 64;
    QTest::newRow("negative") << u"-1"_s << 64;
    QTest::newRow("invalid") << u"abc"_s << 64;
}

void tst_QLottieParse::maxNestingDepthFromEnvironment()
{
#if QT_CONFIG(process)
    if (qEnvironmentVariableIsSet("QLOTTIE_MAX_NESTING_DEPTH"))
        QSKIP("Already running with QLOTTIE_MAX_NESTING_DEPTH set");

    QFETCH(QString, value);
    QFETCH(int, expected);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(u"QLOTTIE_MAX_NESTING_DEPTH"_s, value);
    env.insert(u"TST_QLOTTIEPARSE_EXPECTED_DEPTH"_s, QString::number(expected));

    QProcess process;
    process.setProcessEnvironment(env);
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    process.start(QCoreApplication::applicationFilePath(),
                  { u"maxNestingDepth"_s, u"shapeGroups"_s, u"effectGroups"_s, u"precompChain"_s });
    QVERIFY(process.waitForFinished(60000));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    QCOMPARE(process.exitCode(), 0);
#else
    QSKIP("Requires QProcess");
#endif
}

QJsonObject tst_QLottieParse::transform()
{
    const auto animatable = [](const QJsonValue &value) {
        return QJsonObject{ { "a"_L1, 0 }, { "k"_L1, value } };
    };
    return QJsonObject{ { "o"_L1, animatable(100) },
                        { "r"_L1, animatable(0) },
                        { "p"_L1, animatable(QJsonArray{ 0, 0, 0 }) },
                        { "a"_L1, animatable(QJsonArray{ 0, 0, 0 }) },
                        { "s"_L1, animatable(QJsonArray{ 100, 100, 100 }) } };
}

QJsonObject tst_QLottieParse::shapeLayer(const QJsonArray &shapes)
{
    return QJsonObject{ { "ty"_L1, 4 },         { "ind"_L1, 1 }, { "ip"_L1, 0 },
                        { "op"_L1, 60 },        { "st"_L1, 0 },  { "ks"_L1, transform() },
                        { "shapes"_L1, shapes } };
}

QJsonObject tst_QLottieParse::precompLayer(const QString &refId)
{
    return QJsonObject{ { "ty"_L1, 0 },       { "ind"_L1, 1 }, { "ip"_L1, 0 },
                        { "op"_L1, 60 },      { "st"_L1, 0 },  { "ks"_L1, transform() },
                        { "refId"_L1, refId } };
}

QByteArray tst_QLottieParse::document(const QJsonArray &layers, const QJsonArray &assets)
{
    const QJsonObject root{ { "v"_L1, "5.3.1"_L1 },  { "fr"_L1, 30 },        { "ip"_L1, 0 },
                            { "op"_L1, 60 },         { "w"_L1, 100 },        { "h"_L1, 100 },
                            { "layers"_L1, layers }, { "assets"_L1, assets } };
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

int tst_QLottieParse::groupDepth(const QLottieBase *element)
{
    int depth = 0;
    for (const QLottieBase *child : element->children())
        depth = qMax(depth, groupDepth(child));
    return element->type() == LOTTIE_SHAPE_GROUP_IX ? depth + 1 : depth;
}

void tst_QLottieParse::shapeGroups_data()
{
    QTest::addColumn<int>("nesting");
    QTest::addColumn<bool>("exceeds");

    QTest::newRow("max") << qLottieMaxNestingDepth() << false;
    QTest::newRow("max+1") << qLottieMaxNestingDepth() + 1 << true;
    QTest::newRow("500") << 500 << true;
}

void tst_QLottieParse::shapeGroups()
{
    QFETCH(int, nesting);
    QFETCH(bool, exceeds);

    QJsonArray items{ QJsonObject{
            { "ty"_L1, "rc"_L1 },
            { "s"_L1, QJsonObject{ { "a"_L1, 0 }, { "k"_L1, QJsonArray{ 10, 10 } } } },
            { "p"_L1, QJsonObject{ { "a"_L1, 0 }, { "k"_L1, QJsonArray{ 0, 0 } } } },
            { "r"_L1, QJsonObject{ { "a"_L1, 0 }, { "k"_L1, 0 } } } } };
    for (int i = 0; i < nesting; ++i)
        items = QJsonArray{ QJsonObject{ { "ty"_L1, "gr"_L1 }, { "it"_L1, items } } };

    if (exceeds) {
        QTest::ignoreMessage(QtWarningMsg,
                             QRegularExpression(u"Shape group nesting exceeds maximum depth"_s));
    }

    QLottieRoot root;
    QVERIFY(root.parseSource(document({ shapeLayer(items) }), QUrl()) >= 0);
    QCOMPARE(groupDepth(&root), qMin(nesting, qLottieMaxNestingDepth()));
}

void tst_QLottieParse::effectGroups_data()
{
    QTest::addColumn<int>("nesting");
    QTest::addColumn<bool>("exceeds");

    QTest::newRow("max") << qLottieMaxNestingDepth() << false;
    QTest::newRow("max+1") << qLottieMaxNestingDepth() + 1 << true;
    QTest::newRow("500") << 500 << true;
}

void tst_QLottieParse::effectGroups()
{
    QFETCH(int, nesting);
    QFETCH(bool, exceeds);

    QJsonArray effects;
    for (int i = 0; i < nesting; ++i)
        effects = QJsonArray{ QJsonObject{ { "ty"_L1, 5 }, { "en"_L1, 1 }, { "ef"_L1, effects } } };

    QJsonObject layer = shapeLayer({});
    layer.insert("ef"_L1, effects);

    if (exceeds) {
        QTest::ignoreMessage(QtWarningMsg,
                             QRegularExpression(u"Effect group nesting exceeds maximum depth"_s));
    }

    QLottieRoot root;
    QVERIFY(root.parseSource(document({ layer }), QUrl()) >= 0);
}

void tst_QLottieParse::precompChain_data()
{
    QTest::addColumn<int>("length");
    QTest::addColumn<bool>("exceeds");

    QTest::newRow("max") << qLottieMaxNestingDepth() << false;
    QTest::newRow("max+1") << qLottieMaxNestingDepth() + 1 << true;
    QTest::newRow("1000") << 1000 << true;
}

void tst_QLottieParse::precompChain()
{
    QFETCH(int, length);
    QFETCH(bool, exceeds);

    const auto id = [](int i) { return u"comp_%1"_s.arg(i, 6, 10, QLatin1Char('0')); };

    QJsonArray assets;
    for (int i = 0; i < length - 1; ++i)
        assets.append(QJsonObject{ { "id"_L1, id(i) },
                                   { "layers"_L1, QJsonArray{ precompLayer(id(i + 1)) } } });
    assets.append(QJsonObject{ { "id"_L1, id(length - 1) },
                               { "layers"_L1, QJsonArray{ shapeLayer({}) } } });

    if (exceeds) {
        QTest::ignoreMessage(QtWarningMsg,
                             QRegularExpression(u"Precomp nesting exceeds maximum depth"_s));
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u"Precomp layer resolving failed"_s));
    }

    QLottieRoot root;
    const int ret = root.parseSource(document({ precompLayer(id(0)) }, assets), QUrl());
    QCOMPARE(ret < 0, exceeds);
}

QTEST_MAIN(tst_QLottieParse)

#include "tst_lottieparse.moc"
