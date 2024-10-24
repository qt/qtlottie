// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "private/bmlayer_p.h"
#include "private/bmrect_p.h"

using namespace Qt::StringLiterals;

class tst_BMRect: public QObject
{
    Q_OBJECT

public:
    tst_BMRect();
    ~tst_BMRect();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testStaticInitialX();
    void testStaticInitialY();
    void testStaticInitialWidth();
    void testStaticInitialHeight();
    void testStaticInitialRoundness();
    void testStaticUpdatedX();
    void testStaticUpdatedY();
    void testStaticUpdatedWidth();
    void testStaticUpdatedHeight();
    void testStaticUpdatedRoundness();

    void testAnimatedInitialX();
    void testAnimatedInitialY();
    void testAnimatedInitialWidth();
    void testAnimatedInitialHeight();
    void testAnimatedInitialRoundness();
    void testAnimatedUpdatedX();
    void testAnimatedUpdatedY();
    void testAnimatedUpdatedWidth();
    void testAnimatedUpdatedHeight();
    void testAnimatedUpdatedRoundness();

    void testName();
    void testType();
    void testHidden();
    void testActive();
    void testDirection();

private:
    void loadTestData(const QString &filename);
    void updateProperty(int frame);

    BMRect *m_rect = nullptr;
};

tst_BMRect::tst_BMRect()
{

}

tst_BMRect::~tst_BMRect()
{

}

void tst_BMRect::initTestCase()
{
}

void tst_BMRect::cleanupTestCase()
{
    if (m_rect)
        delete m_rect;
}

void tst_BMRect::testStaticInitialX()
{
    loadTestData("rect_static_30x30_5050_rad0.json");

    QVERIFY(qFuzzyCompare(m_rect->position().x(), 50.0));
}

void tst_BMRect:: testStaticInitialY()
{
    loadTestData("rect_static_30x30_5050_rad0.json");

    QVERIFY(qFuzzyCompare(m_rect->position().y(), 50.0));
}

void tst_BMRect::testStaticInitialWidth()
{
    loadTestData("rect_static_30x30_5050_rad0.json");

    QVERIFY(qFuzzyCompare(m_rect->size().width(), 30.0));
}

void tst_BMRect::testStaticInitialHeight()
{
    loadTestData("rect_static_30x30_5050_rad0.json");

    QVERIFY(qFuzzyCompare(m_rect->size().height(), 30.0));
}

void tst_BMRect:: testStaticInitialRoundness()
{
    loadTestData("rect_static_30x30_5050_rad0.json");

    QVERIFY(qFuzzyCompare(m_rect->roundness(), 0.0));
}

void tst_BMRect::testStaticUpdatedX()
{
    loadTestData("rect_static_30x30_5050_rad0.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->position().x(), 50.0));
}

void tst_BMRect:: testStaticUpdatedY()
{
    loadTestData("rect_static_30x30_5050_rad0.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->position().y(), 50.0));
}

void tst_BMRect::testStaticUpdatedWidth()
{
    loadTestData("rect_static_30x30_5050_rad0.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->size().width(), 30.0));
}

void tst_BMRect::testStaticUpdatedHeight()
{
    loadTestData("rect_static_30x30_5050_rad0.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->size().height(), 30.0));
}

void tst_BMRect:: testStaticUpdatedRoundness()
{
    loadTestData("rect_static_30x30_5050_rad0.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->roundness(), 0.0));
}

void tst_BMRect::testAnimatedInitialX()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_rect->position().x(), 15.0));
}

void tst_BMRect:: testAnimatedInitialY()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_rect->position().y(), 15.0));
}

void tst_BMRect::testAnimatedInitialWidth()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_rect->size().width(), 30.0));
}

void tst_BMRect::testAnimatedInitialHeight()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_rect->size().height(), 30.0));
}

void tst_BMRect::testAnimatedInitialRoundness()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_rect->roundness(), 0.0));
}

void tst_BMRect::testAnimatedUpdatedX()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->position().x(), 75.0));
}

void tst_BMRect:: testAnimatedUpdatedY()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->position().y(), 75.0));
}

void tst_BMRect::testAnimatedUpdatedWidth()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->size().width(), 50.0));
}

void tst_BMRect::testAnimatedUpdatedHeight()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->size().height(), 50.0));
}

void tst_BMRect::testAnimatedUpdatedRoundness()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->roundness(), 25.0));
}

void tst_BMRect::testName()
{
    loadTestData("rect_static_30x30_5050_rad0.json");
    QVERIFY(m_rect->name() == QString("Rectangle Path 1"));
}

void tst_BMRect::testType()
{
    loadTestData("rect_static_30x30_5050_rad0.json");
    QVERIFY(m_rect->type() == BM_SHAPE_RECT_IX);
}

void tst_BMRect::testHidden()
{
    loadTestData("rect_hidden.json");
    QVERIFY(m_rect->hidden() == true);
}

void tst_BMRect::testActive()
{
    loadTestData("rect_hidden.json");
    QVERIFY(m_rect->active(100) == false);

    loadTestData("rect_static_30x30_5050_rad0.json");
    QVERIFY(m_rect->active(100) == true);
}

void tst_BMRect::testDirection()
{
    loadTestData("rect_hidden.json");
    QVERIFY(m_rect->direction() == 0);

    loadTestData("rect_direction.json");
    QVERIFY(m_rect->direction() == 3);
}

void tst_BMRect::loadTestData(const QString &filename)
{
    if (m_rect) {
        delete m_rect;
        m_rect = nullptr;
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
    BMShape* shape = nullptr;
    while (shapesIt != shapes.end()) {
        QJsonObject childObj = (*shapesIt).toObject();
        shape = BMShape::construct(childObj, version);
        QVERIFY(shape != nullptr);
        if (shape->type() == BM_SHAPE_RECT_IX)
            break;
        shapesIt++;
    }

    m_rect = static_cast<BMRect*>(shape);

    QVERIFY(m_rect != nullptr);
}

void tst_BMRect::updateProperty(int frame)
{
    m_rect->updateProperties(frame);
}

QTEST_MAIN(tst_BMRect)
#include "tst_bmrect.moc"
