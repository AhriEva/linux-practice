#pragma once
#include "package.hpp"
#include <QNetworkAccessManager>
#include <QUrl>

/// Общий источник метаданных; готовые средства Cargo не вызываются.
class Repository {
public:
    virtual ~Repository() = default;
    virtual Package select(const QString &name, const QString &requirement) = 0;
};

/// Рассчитать путь Cargo sparse index по длине имени.
QString indexPath(const QString &name);

/// Разобрать JSON Lines index-файл одного crate.
QVector<Package> parseIndex(const QByteArray &data, const QString &name);

/// HTTP-клиент Cargo sparse index с кешем метаданных в памяти.
class CargoRepository : public Repository {
public:
    explicit CargoRepository(QString url);
    Package select(const QString &name, const QString &requirement) override;
private:
    QByteArray download(const QString &path);
    QUrl base_;
    QNetworkAccessManager manager_;
    QMap<QString, QVector<Package>> cache_;
};
