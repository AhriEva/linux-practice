#include "repository.hpp"
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTimer>

namespace {
constexpr int TimeoutMilliseconds = 15000;
constexpr qsizetype MaximumResponseBytes = 32 * 1024 * 1024;

QStringList stringArray(const QJsonValue &value) {
    if (!value.isArray()) throw GraphError("ожидается массив строк в индексе");
    QStringList result;
    for (const auto &item : value.toArray()) {
        if (!item.isString()) throw GraphError("ожидается строка в индексе");
        result << item.toString();
    }
    return result;
}

Dependency readDependency(const QJsonValue &value) {
    if (!value.isObject()) throw GraphError("зависимость должна быть объектом");
    const auto object = value.toObject();
    Dependency dep;
    dep.alias = object.value("name").toString();
    dep.name = object.value("package").isString()
        ? object.value("package").toString() : dep.alias;
    dep.requirement = object.value("req").toString();
    indexPath(dep.name);
    matchesRequirement(parseVersion("0.0.0"), dep.requirement);
    dep.optional = object.value("optional").toBool(false);
    dep.defaultFeatures = object.value("default_features").toBool(true);
    if (object.contains("features")) dep.features = stringArray(object.value("features"));
    if (object.value("kind").isString()) dep.kind = object.value("kind").toString();
    if (dep.kind != "normal" && dep.kind != "build" && dep.kind != "dev")
        throw GraphError("неизвестный kind зависимости");
    dep.registry = object.value("registry").toString();
    return dep;
}

Package readPackage(const QJsonObject &object, const QString &expectedName) {
    Package package;
    package.name = object.value("name").toString();
    package.version = object.value("vers").toString();
    if (package.name.compare(expectedName, Qt::CaseInsensitive) != 0)
        throw GraphError("имя в индексе не совпадает с запросом: " + expectedName);
    parseVersion(package.version);
    if (!object.value("deps").isArray()) throw GraphError("deps должен быть массивом");
    for (const auto &value : object.value("deps").toArray())
        package.dependencies << readDependency(value);
    package.yanked = object.value("yanked").toBool(false);
    for (const auto &field : {QString("features"), QString("features2")}) {
        if (!object.contains(field)) continue;
        if (!object.value(field).isObject()) throw GraphError("features должен быть объектом");
        const auto features = object.value(field).toObject();
        for (auto it = features.begin(); it != features.end(); ++it)
            package.features[it.key()] += stringArray(it.value());
    }
    return package;
}
}

QString indexPath(const QString &name) {
    static const QRegularExpression pattern("^[A-Za-z][A-Za-z0-9_-]{0,63}$");
    if (!pattern.match(name).hasMatch()) throw GraphError("неверное имя crate: " + name);
    const auto lower = name.toLower();
    if (lower.size() == 1) return "1/" + lower;
    if (lower.size() == 2) return "2/" + lower;
    if (lower.size() == 3) return "3/" + lower.left(1) + "/" + lower;
    return lower.left(2) + "/" + lower.mid(2, 2) + "/" + lower;
}

QVector<Package> parseIndex(const QByteArray &data, const QString &name) {
    QVector<Package> packages;
    QSet<QString> versions;
    for (const auto &line : data.split('\n')) {
        if (line.trimmed().isEmpty()) continue;
        QJsonParseError error;
        const auto document = QJsonDocument::fromJson(line, &error);
        if (error.error != QJsonParseError::NoError || !document.isObject())
            throw GraphError("неверный JSON Lines для " + name + ": " + error.errorString());
        const auto object = document.object();
        if (object.value("v").toInt(1) > 2) continue;
        auto package = readPackage(object, name);
        if (versions.contains(package.version)) throw GraphError("повтор версии в индексе");
        versions.insert(package.version);
        packages << package;
    }
    if (packages.isEmpty()) throw GraphError("нет поддерживаемых версий: " + name);
    return packages;
}

CargoRepository::CargoRepository(QString url) {
    if (url.startsWith("sparse+")) url.remove(0, 7);
    if (!url.endsWith('/')) url += '/';
    base_ = QUrl(url);
}

QByteArray CargoRepository::download(const QString &path) {
    QNetworkRequest request(base_.resolved(QUrl(path)));
    request.setHeader(QNetworkRequest::UserAgentHeader, "confPract-pract2/1.0");
    request.setTransferTimeout(TimeoutMilliseconds);
    auto *reply = manager_.get(request);
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, reply, &QNetworkReply::abort);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::readyRead, reply, [reply] {
        if (reply->bytesAvailable() > MaximumResponseBytes) reply->abort();
    });
    timer.start(TimeoutMilliseconds);
    loop.exec();
    const auto data = reply->readAll();
    const auto error = reply->error();
    const auto message = reply->errorString();
    reply->deleteLater();
    if (error != QNetworkReply::NoError)
        throw GraphError("HTTP " + request.url().toString() + ": " + message);
    if (data.size() > MaximumResponseBytes) throw GraphError("слишком большой index-файл");
    return data;
}

Package CargoRepository::select(const QString &name, const QString &requirement) {
    if (!cache_.contains(name)) cache_.insert(name, parseIndex(download(indexPath(name)), name));
    bool found = false;
    Package best;
    for (const auto &package : cache_.value(name)) {
        if (package.yanked || !matchesRequirement(parseVersion(package.version), requirement)) continue;
        if (!found || compareVersions(parseVersion(package.version), parseVersion(best.version)) > 0) {
            best = package;
            found = true;
        }
    }
    if (!found) throw GraphError("нет подходящей версии " + name + " для " + requirement);
    return best;
}
