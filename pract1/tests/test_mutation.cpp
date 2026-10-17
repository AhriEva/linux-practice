#include "shell.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QtTest>

class MutationTests : public QObject {
    Q_OBJECT
private:
    static Shell fixture() {
        Shell shell({QString(FIXTURE_DIR) + "/full.xml", {}, false});
        shell.loadVfs();
        return shell;
    }
    static QByteArray sourceHash() {
        QFile file(QString(FIXTURE_DIR) + "/full.xml");
        if (!file.open(QIODevice::ReadOnly)) throw ShellError("fixture");
        return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha256);
    }
private slots:
    void mkdirModesAndErrors() {
        auto shell = fixture();
        QVERIFY(shell.execute("mkdir -p -m 700 '/a b/c/d'").ok);
        QVERIFY(shell.execute("ls -l '/a b/c'").output.contains("d 700 0 d/"));
        QVERIFY(!shell.execute("mkdir '/a b/c/d'").ok);
        QVERIFY(shell.execute("mkdir -p '/a b/c/d'").ok);
        QVERIFY(!shell.execute("mkdir /absent/child").ok);
        QVERIFY(!shell.execute("mkdir /lines.txt/child").ok);
        QVERIFY(!shell.execute("mkdir -m 999 /bad").ok);
        QVERIFY(!shell.execute("mkdir -m").ok);
        QVERIFY(!shell.execute("mkdir ''").ok);
        QVERIFY(shell.execute("mkdir /x /y").ok);
        QVERIFY(shell.execute("mkdir -- /-name").ok);
        QVERIFY(shell.execute("mkdir -p /").ok);
        QVERIFY(!shell.execute("mkdir /").ok);
    }
    void chmodAndPermissions() {
        auto shell = fixture();
        QVERIFY(shell.execute("chmod 600 /lines.txt").ok);
        QVERIFY(shell.execute("chmod u+x,g-w,o= /lines.txt").ok);
        QVERIFY(shell.execute("ls -l /lines.txt").output.startsWith("- 700"));
        QVERIFY(shell.execute("chmod 000 /lines.txt").ok);
        QVERIFY(!shell.execute("uniq /lines.txt").ok);
        QVERIFY(shell.execute("chmod 644 /lines.txt").ok);
        QVERIFY(shell.execute("uniq /lines.txt").ok);
        QVERIFY(shell.execute("chmod 500 /empty").ok);
        QVERIFY(!shell.execute("mkdir /empty/no").ok);
        QVERIFY(shell.execute("chmod 000 /empty").ok);
        QVERIFY(!shell.execute("cd /empty").ok);
        QVERIFY(shell.execute("chmod 700 /empty").ok);
        QVERIFY(shell.execute("mkdir /empty/yes").ok);
        QVERIFY(!shell.execute("chmod 888 /empty").ok);
        QVERIFY(!shell.execute("chmod u+bad /empty").ok);
    }
    void recursiveAndMemoryOnly() {
        const auto hash = sourceHash();
        auto shell = fixture();
        QVERIFY(shell.execute("chmod -R 700 /home").ok);
        QVERIFY(shell.execute("ls -l /home/student").output.contains("d 700"));
        QVERIFY(shell.execute("mkdir /created").ok);
        auto fresh = fixture();
        QVERIFY(!fresh.execute("cd /created").ok);
        QVERIFY(fresh.execute("ls -l /home").output.contains("d 755"));
        QCOMPARE(sourceHash(), hash);
    }
    void invalidChmodIsAtomic() {
        auto shell = fixture();
        QVERIFY(!shell.execute("chmod 000 /lines.txt /missing").ok);
        QVERIFY(shell.execute("ls -l /lines.txt").output.startsWith("- 644"));
        QVERIFY(!shell.execute("chmod u+x,invalid /lines.txt").ok);
        QVERIFY(shell.execute("ls -l /lines.txt").output.startsWith("- 644"));
    }
};

QTEST_GUILESS_MAIN(MutationTests)
#include "test_mutation.moc"
