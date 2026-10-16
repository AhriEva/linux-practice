#include "repository.hpp"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtTest>

class RepositoryTests : public QObject {
    Q_OBJECT
private:
    static QByteArray row(QString version, bool yanked = false) {
        return QJsonDocument(QJsonObject{
            {"name", "demo"}, {"vers", version}, {"deps", QJsonArray()},
            {"yanked", yanked}
        }).toJson(QJsonDocument::Compact) + '\n';
    }
private slots:
    void paths() {
        QCOMPARE(indexPath("A"), QString("1/a"));
        QCOMPARE(indexPath("ab"), QString("2/ab"));
        QCOMPARE(indexPath("Abc"), QString("3/a/abc"));
        QCOMPARE(indexPath("serde_json"), QString("se/rd/serde_json"));
        QVERIFY_THROWS_EXCEPTION(GraphError, indexPath("../x"));
    }
    void parseAndErrors() {
        QCOMPARE(parseIndex(row("1.0.0"), "demo").size(), 1);
        QVERIFY_THROWS_EXCEPTION(GraphError, parseIndex("bad", "demo"));
        QVERIFY_THROWS_EXCEPTION(GraphError, parseIndex(row("1.0.0"), "other"));
        QVERIFY_THROWS_EXCEPTION(GraphError, parseIndex("{}", "demo"));
        QVERIFY_THROWS_EXCEPTION(GraphError, parseIndex(row("1.0.0") + row("1.0.0"), "demo"));
    }
    void httpSelectionAndCache() {
        QTcpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost, 0));
        int requests = 0;
        const auto body = row("1.2.0") + row("2.0.0") + row("1.9.0", true) + row("1.5.0");
        connect(&server, &QTcpServer::newConnection, this, [&] {
            auto *socket = server.nextPendingConnection();
            connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
            connect(socket, &QTcpSocket::readyRead, socket, [&, socket] {
                const auto request = socket->readAll();
                QVERIFY(request.startsWith("GET /de/mo/demo "));
                ++requests;
                socket->write("HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: "
                              + QByteArray::number(body.size()) + "\r\n\r\n" + body);
                socket->disconnectFromHost();
            });
        });
        CargoRepository repo(QString("http://127.0.0.1:%1/").arg(server.serverPort()));
        QCOMPARE(repo.select("demo", "^1").version, QString("1.5.0"));
        QCOMPARE(repo.select("demo", "*").version, QString("2.0.0"));
        QCOMPARE(requests, 1);
        QVERIFY_THROWS_EXCEPTION(GraphError, repo.select("demo", "^3"));
    }
    void featuresAndRename() {
        Package package;
        package.name = "root";
        package.features["default"] = {"extra"};
        package.features["extra"] = {"dep:alias", "alias/child"};
        Dependency dep;
        dep.alias = "alias";
        dep.name = "real";
        dep.optional = true;
        package.dependencies << dep;
        const auto active = activeDependencies(package, {});
        QCOMPARE(active.size(), 1);
        QCOMPARE(active[0].dependency.name, QString("real"));
        QVERIFY(active[0].request.names.contains("child"));
        QCOMPARE(activeDependencies(package, {false, {}}).size(), 0);
    }
};
QTEST_GUILESS_MAIN(RepositoryTests)
#include "test_repository.moc"
