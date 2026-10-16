#include "shell.hpp"
#include <QtTest>

class ShellTests : public QObject {
    Q_OBJECT
private:
    static Shell fixture() {
        Shell shell({QString(FIXTURE_DIR) + "/full.xml", {}, false});
        shell.loadVfs();
        return shell;
    }
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
    void navigationAndListing() {
        auto shell = fixture();
        QVERIFY(!shell.execute("ls").output.contains(".hidden"));
        QVERIFY(shell.execute("ls -a").output.contains(".hidden"));
        QVERIFY(shell.execute("ls -l /lines.txt").output.startsWith("- 644"));
        QVERIFY(shell.execute("cd /home/student").ok);
        QCOMPARE(shell.cwd(), QString("/home/student"));
        QVERIFY(shell.execute("cd 'folder with spaces'").ok);
        QVERIFY(shell.execute("ls 'a b.txt'").ok);
        QVERIFY(!shell.execute("cd /lines.txt").ok);
        QVERIFY(!shell.execute("cd /missing").ok);
        QCOMPARE(shell.cwd(), QString("/home/student/folder with spaces"));
        QVERIFY(shell.execute("cd ../../../../..").ok);
        QCOMPARE(shell.cwd(), QString("/"));
        QVERIFY(!shell.execute("ls -z").ok);
        QVERIFY(!shell.execute("ls a b").ok);
    }
    void treeAndIdentity() {
        auto shell = fixture();
        const auto tree = shell.execute("tree /home").output;
        QVERIFY(tree.contains("documents"));
        QVERIFY(tree.contains("notes.txt"));
        QVERIFY(!shell.execute("tree -L 1 /home").output.contains("notes"));
        QVERIFY(!shell.execute("tree -d").output.contains("lines.txt"));
        QVERIFY(shell.execute("tree -a").output.contains(".hidden"));
        QVERIFY(!shell.execute("tree -L 0").ok);
        QVERIFY(!shell.execute("tree -L").ok);
        QVERIFY(!shell.execute("whoami extra").ok);
        QVERIFY(!shell.execute("whoami").output.isEmpty());
    }
    void adjacentDuplicates() {
        auto shell = fixture();
        QCOMPARE(shell.execute("uniq /lines.txt").output,
                 QString("alpha\nbeta\ngamma\nalpha\n"));
        QCOMPARE(shell.execute("uniq -d /lines.txt").output,
                 QString("alpha\nbeta\n"));
        QCOMPARE(shell.execute("uniq -u /lines.txt").output,
                 QString("gamma\nalpha\n"));
        QCOMPARE(shell.execute("uniq -c /lines.txt").output,
                 QString("      2 alpha\n      2 beta\n"
                         "      1 gamma\n      1 alpha\n"));
        QVERIFY(!shell.execute("uniq -du /lines.txt").ok);
        QVERIFY(!shell.execute("uniq /binary.bin").ok);
        QVERIFY(!shell.execute("uniq /home").ok);
        QVERIFY(!shell.execute("uniq").ok);
    }

};

QTEST_GUILESS_MAIN(ShellTests)
#include "test_shell.moc"
