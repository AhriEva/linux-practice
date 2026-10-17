#include "config.hpp"
#include "error.hpp"
#include <QTemporaryFile>
#include <QtTest>

class ConfigTests : public QObject {
    Q_OBJECT
private slots:
    void defaultsAndDump() {
        Config config;
        config.package = "getrandom@0.2.16";
        validateConfig(config);
        QVERIFY(config.dump().contains("max-depth=5"));
        QVERIFY(config.dump().contains("test-mode=false"));
    }
    void errors() {
        Config config;
        QVERIFY_THROWS_EXCEPTION(GraphError, validateConfig(config));
        config.package = "../bad";
        QVERIFY_THROWS_EXCEPTION(GraphError, validateConfig(config));
        config.package = "itoa";
        config.maxDepth = -1;
        QVERIFY_THROWS_EXCEPTION(GraphError, validateConfig(config));
        config.maxDepth = 0;
        config.repository = "not-url";
        QVERIFY_THROWS_EXCEPTION(GraphError, validateConfig(config));
        config.repository = "https://index.crates.io/";
        config.exclude = "itoa";
        QVERIFY_THROWS_EXCEPTION(GraphError, validateConfig(config));
        config.exclude = "\n";
        QVERIFY_THROWS_EXCEPTION(GraphError, validateConfig(config));
    }
    void testFile() {
        QTemporaryFile file;
        QVERIFY(file.open());
        Config config{"A", file.fileName(), true, true, 0, {}};
        validateConfig(config);
        config.package = "lowercase";
        QVERIFY_THROWS_EXCEPTION(GraphError, validateConfig(config));
        config.package = "A";
        config.repository = "absent.json";
        QVERIFY_THROWS_EXCEPTION(GraphError, validateConfig(config));
    }
};
QTEST_GUILESS_MAIN(ConfigTests)
#include "test_config.moc"
