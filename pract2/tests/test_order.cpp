#include "order.hpp"
#include "test_repository.hpp"
#include <QtTest>

class OrderTests : public QObject {
    Q_OBJECT
private:
    static Graph fixture(QString file, int depth = 5) {
        Config config{"A", QString(FIXTURE_DIR) + "/" + file + ".json", true, false, depth, {}};
        TestRepository repository(config.repository);
        return buildGraph(repository, config);
    }
private slots:
    void dependenciesFirst() {
        const auto graph = fixture("diamond");
        const auto order = loadOrder(graph);
        QCOMPARE(order, QStringList({"E", "D", "B", "C", "A"}));
        for (auto it = graph.edges.begin(); it != graph.edges.end(); ++it)
            for (const auto &child : it.value())
                QVERIFY(order.indexOf(child) < order.indexOf(it.key()));
    }
    void cycleIsError() {
        QVERIFY_THROWS_EXCEPTION(GraphError, loadOrder(fixture("cycle")));
    }
    void isolatedAndLimited() {
        QCOMPARE(loadOrder(fixture("isolated")), QStringList({"A"}));
        QCOMPARE(loadOrder(fixture("simple", 0)), QStringList({"A"}));
    }
};
QTEST_GUILESS_MAIN(OrderTests)
#include "test_order.moc"
