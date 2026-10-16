#include "vfs.hpp"
#include <QTemporaryFile>
#include <QtTest>

class VfsTests : public QObject {
    Q_OBJECT
private:
    static void loadXml(Vfs &vfs, const QByteArray &xml) {
        QTemporaryFile file;
        if (!file.open()) throw ShellError("ошибка временного файла теста");
        file.write(xml);
        file.flush();
        vfs.load(file.fileName());
    }
private slots:
    void loadAndBinary() {
        Vfs vfs;
        vfs.load(QString(FIXTURE_DIR) + "/full.xml");
        QVERIFY(vfs.motd().contains("18"));
        QCOMPARE(vfs.resolve("/binary.bin")->data.toHex(),
                 QByteArray("00010203fffe"));
        QCOMPARE(vfs.readText("/home/student/documents/notes.txt"),
                 QString("Один\nОдин\nДва\nТри\nТри\n"));
        QVERIFY_THROWS_EXCEPTION(ShellError, vfs.readText("/binary.bin"));
    }
    void badXml_data() {
        QTest::addColumn<QByteArray>("xml");
        QTest::newRow("syntax") << QByteArray("<vfs><dir></vfs>");
        QTest::newRow("root") << QByteArray("<wrong/>");
        QTest::newRow("name") << QByteArray("<vfs><dir name='..'/></vfs>");
        QTest::newRow("duplicate") << QByteArray(
            "<vfs><dir name='x'/><file name='x'/></vfs>");
        QTest::newRow("base64") << QByteArray(
            "<vfs><file name='x' encoding='base64'>%%%</file></vfs>");
        QTest::newRow("mode") << QByteArray(
            "<vfs><dir name='x' mode='999'/></vfs>");
        QTest::newRow("nested-file") << QByteArray(
            "<vfs><file name='x'><dir name='y'/></file></vfs>");
        QTest::newRow("doctype") << QByteArray(
            "<!DOCTYPE vfs><vfs/>");
    }
    void badXml() {
        QFETCH(QByteArray, xml);
        Vfs vfs;
        QVERIFY_THROWS_EXCEPTION(ShellError, loadXml(vfs, xml));
    }
    void atomicLoadAndMissing() {
        Vfs vfs;
        loadXml(vfs, "<vfs><file name='x'>ok</file></vfs>");
        QVERIFY_THROWS_EXCEPTION(ShellError, loadXml(vfs, "<vfs>"));
        QCOMPARE(vfs.readText("/x"), QString("ok"));
        QVERIFY_THROWS_EXCEPTION(ShellError, vfs.load("missing.xml"));
    }
    void pathNormalization() {
        QCOMPARE(absolutePath(".././../x", "/home/student"), QString("/x"));
        QCOMPARE(absolutePath("../../..", "/"), QString("/"));
        QCOMPARE(absolutePath("/a//b/", "/x"), QString("/a/b"));
    }
};

QTEST_GUILESS_MAIN(VfsTests)
#include "test_vfs.moc"
