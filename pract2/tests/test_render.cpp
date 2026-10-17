#include "render.hpp"
#include "test_repository.hpp"
#include <QtTest>

class RenderTests : public QObject {
    Q_OBJECT
private:
    static Graph fixture(QString name, int depth = 5) {
        Config config{"A", QString(FIXTURE_DIR) + "/" + name + ".json", true, false, depth, {}};
        TestRepository repo(config.repository);
        return buildGraph(repo, config);
    }
private slots:
    void asciiOnlyAndShared() {
        const auto graph = fixture("diamond");
        const auto output = asciiTree(graph);
        QCOMPARE(output, QString("A\n|-- B\n|   `-- D\n|       `-- E\n`-- C\n    `-- D (*)"));
        for (const auto ch : output) QVERIFY(ch.unicode() < 128);
    }
    void cyclesAndLimits() {
        const auto graph = fixture("cycle", 20);
        QVERIFY(asciiTree(graph).contains("A [cycle]"));
        QVERIFY(asciiTree(fixture("simple", 0)).contains("A [depth limit]"));
    }
    void plantUmlEdges() {
        const auto output = plantUml(fixture("diamond"));
        QVERIFY(output.startsWith("@startuml\n"));
        QVERIFY(output.endsWith("@enduml"));
        QCOMPARE(output.count("component \""), 5);
        QCOMPARE(output.count(" --> "), 5);
        QVERIFY(output.contains("n1 --> n3"));
        QVERIFY(output.contains("n2 --> n3"));
        QVERIFY(plantUml(fixture("simple", 0)).contains("<<depth_limit>>"));
        QCOMPARE(plantUml(fixture("isolated")).count(" --> "), 0);
    }
};
QTEST_GUILESS_MAIN(RenderTests)
#include "test_render.moc"
