#include "graph.hpp"
#include "test_repository.hpp"
#include <QTemporaryFile>
#include <QtTest>

namespace {
class MemoryRepository : public Repository {
public:
    QMap<QString, Package> packages;
    QStringList queries;
    Package select(const QString &name, const QString &) override {
        queries << name;
        return packages.value(name);
    }
    void add(const QString &name, const QStringList &dependencies) {
        Package package;
        package.name = name;
        for (const auto &child : dependencies) {
            Dependency dependency;
            dependency.alias = child;
            dependency.name = child;
            dependency.requirement = "*";
            package.dependencies << dependency;
        }
        packages[name] = package;
    }
};
}

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
    void lateFeatureUnionAndPruning() {
        MemoryRepository repo;
        repo.add("A", {"B", "C"});
        repo.add("B", {"D"});
        repo.add("C", {"E"});
        repo.add("D", {"F"});
        repo.add("E", {"D"});
        repo.add("F", {});
        repo.packages["D"].dependencies[0].optional = true;
        repo.packages["D"].features["extra"] = {"dep:F"};
        repo.packages["E"].dependencies[0].features = {"extra"};
        Config params{"A", {}, true, false, 5, {}};
        const auto graph = buildGraph(repo, params);
        QVERIFY(graph.edges.value("D").contains("F"));
        QCOMPARE(graph.depth.value("F"), 3);
        repo.queries.clear();
        params.exclude = "D";
        QVERIFY(!buildGraph(repo, params).packages.contains("D"));
        QVERIFY(!repo.queries.contains("D"));
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
