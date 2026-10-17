#include "version.hpp"
#include <QtTest>

class VersionTests : public QObject {
    Q_OBJECT
private slots:
    void requirements_data() {
        QTest::addColumn<QString>("version");
        QTest::addColumn<QString>("requirement");
        QTest::addColumn<bool>("expected");
        QTest::newRow("caret") << "1.9.0" << "^1.2" << true;
        QTest::newRow("major") << "2.0.0" << "1.2" << false;
        QTest::newRow("zero") << "0.3.0" << "0.2" << false;
        QTest::newRow("zero-short") << "0.8.0" << "0" << true;
        QTest::newRow("zero-two") << "0.0.9" << "0.0" << true;
        QTest::newRow("zero-patch") << "0.0.4" << "0.0.3" << false;
        QTest::newRow("tilde") << "1.2.9" << "~1.2.3" << true;
        QTest::newRow("tilde-fail") << "1.3.0" << "~1.2" << false;
        QTest::newRow("wildcard") << "2.4.9" << "2.*" << true;
        QTest::newRow("range") << "1.4.0" << ">= 1.2, <1.5" << true;
        QTest::newRow("partial-greater") << "1.2.9" << ">1.2" << false;
        QTest::newRow("partial-le") << "1.2.9" << "<=1.2" << true;
        QTest::newRow("partial-eq") << "1.2.9" << "=1.2" << true;
        QTest::newRow("exact") << "1.2.4" << "=1.2.3" << false;
        QTest::newRow("pre-default") << "1.0.0-beta.1" << "*" << false;
        QTest::newRow("pre-opt-in") << "1.0.0-beta.2" << "^1.0.0-beta.1" << true;
        QTest::newRow("pre-other") << "1.1.0-beta" << "^1.0.0-beta" << false;
    }
    void requirements() {
        QFETCH(QString, version);
        QFETCH(QString, requirement);
        QFETCH(bool, expected);
        QCOMPARE(matchesRequirement(parseVersion(version), requirement), expected);
    }
    void orderingAndErrors() {
        QVERIFY(compareVersions(parseVersion("1.0.0-alpha.9"),
                                parseVersion("1.0.0-alpha.10")) < 0);
        QVERIFY(compareVersions(parseVersion("1.0.0"), parseVersion("1.0.0-rc")) > 0);
        QCOMPARE(compareVersions(parseVersion("1.0.0+a"), parseVersion("1.0.0+b")), 0);
        QVERIFY_THROWS_EXCEPTION(GraphError, parseVersion("1.0"));
        QVERIFY_THROWS_EXCEPTION(GraphError, parseVersion("1.0.0-01"));
        QVERIFY_THROWS_EXCEPTION(GraphError, matchesRequirement(parseVersion("1.0.0"), "bad"));
        QVERIFY_THROWS_EXCEPTION(GraphError, matchesRequirement(parseVersion("1.0.0"), ""));
    }
};
QTEST_GUILESS_MAIN(VersionTests)
#include "test_version.moc"
