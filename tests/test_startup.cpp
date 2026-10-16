#include "startup.hpp"
#include <QTemporaryFile>
#include <QtTest>

class StartupTests : public QObject {
    Q_OBJECT
private slots:
    void commentsErrorsAndExit() {
        QTemporaryFile file;
        QVERIFY(file.open());
        file.write("// comment\nconf-dump\nunknown\nwhoami\nexit\nunknown\n");
        file.flush();
        Shell shell;
        QStringList output;
        QVERIFY(!runStartup(shell, file.fileName(), [&output](const QString &s) {
            output << s;
        }));
        QVERIFY(shell.closed());
        const auto transcript = output.join('\n');
        QVERIFY(transcript.contains("/$ // comment"));
        QVERIFY(transcript.contains("vfs="));
        QVERIFY(transcript.contains("строка 3"));
        QCOMPARE(transcript.count("/$ unknown"), 1);
        QVERIFY(transcript.contains("/$ whoami"));
    }
    void missingAndEmptyScript() {
        Shell shell;
        QString error;
        QVERIFY(!runStartup(shell, "absent-script", [&error](const QString &s) {
            error = s;
        }));
        QVERIFY(error.contains("Ошибка скрипта"));
        QVERIFY(runStartup(shell, "", [](const QString &) {}));
    }
    void configurationDump() {
        Shell shell({"a b.xml", "script.txt", true});
        const auto dump = shell.execute("conf-dump");
        QVERIFY(dump.ok);
        QCOMPARE(dump.output,
                 QString("vfs=a b.xml\nstartup=script.txt\nheadless=true"));
        QVERIFY(!shell.execute("conf-dump extra").ok);
    }
};

QTEST_GUILESS_MAIN(StartupTests)
#include "test_startup.moc"
