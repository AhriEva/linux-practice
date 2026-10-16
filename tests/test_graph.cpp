#include "graph.hpp"
#include "test_repository.hpp"
#include <QTemporaryFile>
#include <QtTest>

class GraphTests : public QObject {
    Q_OBJECT
private:
    static Config config(const QString &name, int depth = 5, QString exclude = {}) {
        return {"A", QString(FIXTURE_DIR) + "/" + name + ".json", true, false, depth, exclude};
    }
    static Graph fixture(const Config &config) {
        TestRepository repository(config.repository);
        return buildGraph(repository, config);
    }
private slots:
    void bfsAndDepth() {
        auto graph = fixture(config("simple"));
        QCOMPARE(graph.packages.size(), 5);
        QCOMPARE(graph.depth.value("D"), 2);
        graph = fixture(config("simple", 1));
        QCOMPARE(graph.packages.size(), 3);
        QVERIFY(graph.truncated.contains("B"));
        QVERIFY(!graph.packages.contains("D"));
        graph = fixture(config("simple", 0));
        QCOMPARE(graph.packages.size(), 1);
        QVERIFY(graph.truncated.contains("A"));
    }
    void diamondCycleAndFilter() {
        auto graph = fixture(config("diamond"));
        QCOMPARE(graph.packages.size(), 5);
        QVERIFY(graph.edges.value("B").contains("D"));
        QVERIFY(graph.edges.value("C").contains("D"));
        QCOMPARE(graph.depth.value("D"), 2);
        graph = fixture(config("cycle", 20));
        QCOMPARE(graph.packages.size(), 4);
        QVERIFY(graph.edges.value("C").contains("A"));
        graph = fixture(config("simple", 5, "B"));
        QVERIFY(!graph.packages.contains("B"));
        QVERIFY(!graph.packages.contains("D"));
        QVERIFY(graph.packages.contains("E"));
    }
    void isolatedAndMissing() {
        QCOMPARE(fixture(config("isolated")).packages.size(), 1);
        auto params = config("simple");
        params.package = "Z";
        QVERIFY_THROWS_EXCEPTION(GraphError, fixture(params));
    }
    void invalidTestRepository_data() {
        QTest::addColumn<QByteArray>("data");
        QTest::newRow("json") << QByteArray("no");
        QTest::newRow("empty") << QByteArray("{}");
        QTest::newRow("name") << QByteArray("{\"a\":[]}");
        QTest::newRow("missing") << QByteArray("{\"A\":[\"B\"]}");
        QTest::newRow("type") << QByteArray("{\"A\":1}");
    }
    void invalidTestRepository() {
        QFETCH(QByteArray, data);
        QTemporaryFile file;
        QVERIFY(file.open());
        file.write(data);
        file.flush();
        QVERIFY_THROWS_EXCEPTION(GraphError, TestRepository(file.fileName()));
    }
};
QTEST_GUILESS_MAIN(GraphTests)
#include "test_graph.moc"
