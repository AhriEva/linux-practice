#include "test_repository.hpp"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

TestRepository::TestRepository(const QString &filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        throw GraphError("не удалось открыть тестовый репозиторий: " + file.errorString());
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject() || doc.object().isEmpty())
        throw GraphError("тестовый репозиторий должен быть непустым JSON-объектом");
    const auto object = doc.object();
    const QRegularExpression namePattern("^[A-Z]+$");
    for (auto it = object.begin(); it != object.end(); ++it) {
        if (!namePattern.match(it.key()).hasMatch() || !it.value().isArray())
            throw GraphError("ожидаются имена A..Z и массивы зависимостей");
        Package package;
        package.name = it.key();
        QSet<QString> unique;
        for (const auto &value : it.value().toArray()) {
            const auto name = value.toString();
            if (!value.isString() || !object.contains(name))
                throw GraphError("зависимость не объявлена: " + name);
            if (unique.contains(name)) continue;
            unique.insert(name);
            Dependency dependency;
            dependency.name = name;
            dependency.alias = name;
            dependency.requirement = "*";
            package.dependencies << dependency;
        }
        packages_.insert(package.name, package);
    }
}

Package TestRepository::select(const QString &name, const QString &requirement) {
    Q_UNUSED(requirement);
    if (!packages_.contains(name)) throw GraphError("пакет отсутствует в тестовом графе: " + name);
    return packages_.value(name);
}
