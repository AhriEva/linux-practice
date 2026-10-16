#include "shell.hpp"
#include <QtTest>

class ShellTests : public QObject {
    Q_OBJECT
private slots:
    void quotes() {
        QCOMPARE(parseLine("ls \"a b\" 'c#d' // comment"),
                 QStringList({"ls", "a b", "c#d"}));
        QCOMPARE(parseLine("ls '' a\\ b"),
                 QStringList({"ls", "", "a b"}));
        QVERIFY_THROWS_EXCEPTION(ShellError, parseLine("ls 'bad"));
    }
    void errorsAndExit() {
        Shell shell;
        QVERIFY(!shell.execute("unknown").ok);
        QVERIFY(!shell.execute("exit extra").ok);
        QVERIFY(!shell.closed());
        QVERIFY(shell.execute("exit").ok);
        QVERIFY(shell.closed());
    }
};

QTEST_GUILESS_MAIN(ShellTests)
#include "test_shell.moc"
