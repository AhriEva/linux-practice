#include <QProcess>
#include <QtTest>

class CliTests : public QObject {
    Q_OBJECT
private:
    static QStringList base() {
        return {"--package", "A", "--repository", QString(FIXTURE_DIR) + "/simple.json", "--test-mode"};
    }
private slots:
    void errors_data() {
        QTest::addColumn<QStringList>("args");
        QTest::newRow("package") << QStringList({"--package", "../bad"});
        QTest::newRow("version") << QStringList({"--package", "itoa@bad"});
        QTest::newRow("repository") << QStringList({"--package", "itoa", "--repository", "file:///x"});
        QTest::newRow("test-mode") << (base() << "--test-mode=true");
        QTest::newRow("ascii") << (base() << "--ascii=false");
        QTest::newRow("depth") << (base() << "--max-depth" << "-1");
        QTest::newRow("not-number") << (base() << "--max-depth" << "abc");
        QTest::newRow("overflow") << (base() << "--max-depth" << "9999999999999999");
        QTest::newRow("exclude") << (base() << "--exclude");
        QTest::newRow("excluded-root") << (base() << "--exclude" << "A");
        QTest::newRow("order") << (base() << "--order=yes");
        QTest::newRow("unknown") << (base() << "--unknown");
    }
    void errors() {
        QFETCH(QStringList, args);
        QProcess process;
        process.start(APP_PATH, args);
        QVERIFY(process.waitForFinished());
        QCOMPARE(process.exitStatus(), QProcess::NormalExit);
        QCOMPARE(process.exitCode(), 1);
        QVERIFY(process.readAllStandardError().contains("Ошибка:"));
    }
    void outputAndCycle() {
        QProcess process;
        process.start(APP_PATH, base() << "--ascii" << "--order");
        QVERIFY(process.waitForFinished());
        QCOMPARE(process.exitCode(), 0);
        const auto output = process.readAllStandardOutput();
        QVERIFY(output.startsWith("@startuml\n"));
        QVERIFY(output.contains("ASCII tree:"));
        QVERIFY(output.contains("Порядок загрузки"));
        QVERIFY(!output.contains("package="));
        process.start(APP_PATH, {"--package", "A", "--repository",
                      QString(FIXTURE_DIR) + "/cycle.json", "--test-mode", "--order", "--ascii"});
        QVERIFY(process.waitForFinished());
        QCOMPARE(process.exitCode(), 1);
        QVERIFY(process.readAllStandardOutput().contains("[cycle]"));
        QVERIFY(process.readAllStandardError().contains("цикл"));
    }
};
QTEST_GUILESS_MAIN(CliTests)
#include "test_cli.moc"
