// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "private/qlottielayer_p.h"
#include "private/qlottietrimpath_p.h"

using namespace Qt::StringLiterals;

class tst_QLottieQTrimPath: public QObject
{
    Q_OBJECT

public:
    tst_QLottieQTrimPath();
    ~tst_QLottieQTrimPath();

private:

    //    void testParseStaticRect();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testStaticInitialStart();
    void testStaticInitialEnd();
    void testStaticInitialOffset();
    void testStaticInitialSimultaneous();
    void testStaticUpdatedStart();
    void testStaticUpdatedEnd();
    void testStaticUpdatedOffset();
    void testStaticUpdatedSimultaneous();

    void testAnimatedInitialStart();
    void testAnimatedInitialEnd();
    void testAnimatedInitialOffset();
    void testAnimatedInitialSimultaneous();
    void testAnimatedUpdatedStart();
    void testAnimatedUpdatedEnd();
    void testAnimatedUpdatedOffset();
    void testAnimatedUpdatedSimultaneous();

    void testName();
    void testType();
    void testHidden();
    void testActive();

private:
    void loadTestData(const QString &filename);
    void updateProperty(int frame);

    QLottieQTrimPath *m_trimpath = nullptr;
};

tst_QLottieQTrimPath::tst_QLottieQTrimPath()
{

}

tst_QLottieQTrimPath::~tst_QLottieQTrimPath()
{

}

void tst_QLottieQTrimPath::initTestCase()
{
}

void tst_QLottieQTrimPath::cleanupTestCase()
{
    if (m_trimpath)
        delete m_trimpath;
}

void tst_QLottieQTrimPath::testStaticInitialStart()
{
    loadTestData("trimpath_static_20to80.json");

    QVERIFY(qFuzzyCompare(m_trimpath->start(), 20.0));
}

void tst_QLottieQTrimPath::testStaticInitialEnd()
{
    loadTestData("trimpath_static_20to80.json");

    QVERIFY(qFuzzyCompare(m_trimpath->end(), 80.0));
}

void tst_QLottieQTrimPath::testStaticInitialOffset()
{
    loadTestData("trimpath_static_20to80.json");

    QVERIFY(qFuzzyCompare(m_trimpath->offset(), 0.0));
}

void tst_QLottieQTrimPath::testStaticInitialSimultaneous()
{
    loadTestData("trimpath_static_20to80.json");

    QVERIFY(m_trimpath->simultaneous() == true);
}

void tst_QLottieQTrimPath::testStaticUpdatedStart()
{
    loadTestData("trimpath_static_20to80.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_trimpath->start(), 20.0));
}

void tst_QLottieQTrimPath::testStaticUpdatedEnd()
{
    loadTestData("trimpath_static_20to80.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_trimpath->end(), 80.0));
}

void tst_QLottieQTrimPath::testStaticUpdatedOffset()
{
    loadTestData("trimpath_static_20to80.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_trimpath->offset(), 0.0));
}

void tst_QLottieQTrimPath::testStaticUpdatedSimultaneous()
{
    loadTestData("trimpath_static_20to80.json");
    updateProperty(180);

    QVERIFY(m_trimpath->simultaneous() == true);
}

void tst_QLottieQTrimPath::testAnimatedInitialStart()
{
    loadTestData("trimpath_animated_2080_0_to_0060_3x30.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_trimpath->start(), 20.0));
}

void tst_QLottieQTrimPath::testAnimatedInitialEnd()
{
    loadTestData("trimpath_animated_2080_0_to_0060_3x30.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_trimpath->end(), 80.0));
}

void tst_QLottieQTrimPath::testAnimatedInitialOffset()
{
    loadTestData("trimpath_animated_2080_0_to_0060_3x30.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_trimpath->offset(), 0.0));
}

void tst_QLottieQTrimPath::testAnimatedInitialSimultaneous()
{
    loadTestData("trimpath_animated_2080_0_to_0060_3x30.json");
    updateProperty(0);

    QVERIFY(m_trimpath->simultaneous() == false);
}

void tst_QLottieQTrimPath::testAnimatedUpdatedStart()
{
    loadTestData("trimpath_animated_2080_0_to_0060_3x30.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_trimpath->start(), 0.0));
}

void tst_QLottieQTrimPath::testAnimatedUpdatedEnd()
{
    loadTestData("trimpath_animated_2080_0_to_0060_3x30.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_trimpath->end(), 60.0));
}

void tst_QLottieQTrimPath::testAnimatedUpdatedOffset()
{
    loadTestData("trimpath_animated_2080_0_to_0060_3x30.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_trimpath->offset(), (360 * 3 + 30.0)));
}

void tst_QLottieQTrimPath::testAnimatedUpdatedSimultaneous()
{
    loadTestData("trimpath_animated_2080_0_to_0060_3x30.json");
    updateProperty(180);

    QVERIFY(m_trimpath->simultaneous() == false);
}

void tst_QLottieQTrimPath::testName()
{
    loadTestData("trimpath_hidden.json");

    QVERIFY(m_trimpath->name() == QString("Trim Paths 1"));
}

void tst_QLottieQTrimPath::testType()
{
    loadTestData("trimpath_hidden.json");

    QVERIFY(m_trimpath->type() == LOTTIE_SHAPE_TRIM_IX);
}

void tst_QLottieQTrimPath::testHidden()
{
    loadTestData("trimpath_hidden.json");

    QVERIFY(m_trimpath->hidden() == true);
}

void tst_QLottieQTrimPath::testActive()
{
    loadTestData("trimpath_animated_2080_0_to_0060_3x30.json");
    QVERIFY(m_trimpath->active(100) == true);

    loadTestData("trimpath_hidden.json");
    QVERIFY(m_trimpath->active(100) == false);
}

void tst_QLottieQTrimPath::loadTestData(const QString &filename)
{
    if (m_trimpath) {
        delete m_trimpath;
        m_trimpath = nullptr;
    }

    QFile sourceFile(QFINDTESTDATA(QLatin1String("data/") + filename));
    if (!sourceFile.exists())
        QFAIL("File does not exist");
    if (!sourceFile.open(QIODevice::ReadOnly))
        QFAIL("Cannot read test file");

    QByteArray json = sourceFile.readAll();

    sourceFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(json);
    QJsonObject rootObj = doc.object();
    if (rootObj.empty())
        QFAIL("Cannot parse test file");

    const auto version = QVersionNumber::fromString(rootObj.value("v"_L1).toString());

    QJsonArray layers = rootObj.value(QLatin1String("layers")).toArray();
    QJsonObject layerObj = layers[0].toObject();
    int type = layerObj.value(QLatin1String("ty")).toInt();
    if (type != 4)
        QFAIL("It's not shape layer");

    QJsonArray shapes = layerObj.value(QLatin1String("shapes")).toArray();
    QJsonArray::const_iterator shapesIt = shapes.constBegin();
    QLottieShape* shape = nullptr;
    while (shapesIt != shapes.end()) {
        QJsonObject childObj = (*shapesIt).toObject();
        shape = QLottieShape::construct(childObj, version);
        QVERIFY(shape != nullptr);
        if (shape->type() == LOTTIE_SHAPE_TRIM_IX)
            break;
        shapesIt++;
    }

    m_trimpath = static_cast<QLottieQTrimPath*>(shape);

    QVERIFY(m_trimpath != nullptr);
}

void tst_QLottieQTrimPath::updateProperty(int frame)
{
    m_trimpath->updateProperties(frame);
}

QTEST_MAIN(tst_QLottieQTrimPath)
#include "tst_lottietrimpath.moc"
