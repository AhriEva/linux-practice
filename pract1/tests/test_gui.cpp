#include "terminal.hpp"
#include "identity.hpp"
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSysInfo>
#include <QtTest>

class GuiTests : public QObject {
    Q_OBJECT
private slots:
    void interactiveSession() {
        Shell shell({QString(FIXTURE_DIR) + "/full.xml", {}, false});
        shell.loadVfs();
        Terminal terminal(shell);
        terminal.write(shell.motd());
        terminal.show();
        QVERIFY(terminal.windowTitle().contains(currentUser()));
        QVERIFY(terminal.windowTitle().contains(QSysInfo::machineHostName()));
        auto *input = terminal.findChild<QLineEdit *>("commandInput");
        auto *output = terminal.findChild<QPlainTextEdit *>("transcript");
        QVERIFY(input);
        QVERIFY(output && output->isReadOnly());
        const QStringList commands = {
            "ls -al", "cd /home/student", "tree -L 2", "cd /",
            "uniq -c /lines.txt", "mkdir -p '/demo/new folder'",
            "chmod -R 700 /demo", "tree /demo", "unknown"
        };
        for (const auto &command : commands) {
            QTest::keyClicks(input, command);
            QTest::keyClick(input, Qt::Key_Return);
        }
        QVERIFY(output->toPlainText().contains("new folder"));
        QVERIFY(output->toPlainText().contains("Ошибка:"));
        QVERIFY(input->text().isEmpty());
        const auto imagePath = qEnvironmentVariable("GUI_SCREENSHOT");
        if (!imagePath.isEmpty()) QVERIFY(terminal.grab().save(imagePath));
        QTest::keyClicks(input, "exit");
        QTest::keyClick(input, Qt::Key_Return);
        QTRY_VERIFY(!terminal.isVisible());
        QVERIFY(shell.closed());
    }
};

QTEST_MAIN(GuiTests)
#include "test_gui.moc"
