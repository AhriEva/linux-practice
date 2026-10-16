#pragma once
#include "version.hpp"
#include <QMap>
#include <QSet>
#include <QVector>

/// Прямая зависимость из записи Cargo sparse index.
struct Dependency {
    QString alias;
    QString name;
    QString requirement;
    bool optional = false;
    bool defaultFeatures = true;
    QStringList features;
    QString kind = "normal";
    QString registry;
};

/// Метаданные конкретной опубликованной версии.
struct Package {
    QString name;
    QString version;
    QVector<Dependency> dependencies;
    QMap<QString, QStringList> features;
    bool yanked = false;
    QString id() const { return version.isEmpty() ? name : name + "@" + version; }
};

/// Запрос на активацию default и именованных Cargo features.
struct FeatureRequest {
    bool defaults = true;
    QSet<QString> names;
};

/// Зависимость и передаваемые ей требования features.
struct ActiveDependency {
    Dependency dependency;
    FeatureRequest request;
};

/// Найти обычные/build зависимости с учётом features; dev исключаются.
QVector<ActiveDependency> activeDependencies(const Package &package,
                                            const FeatureRequest &request);
