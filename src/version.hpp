#pragma once
#include "error.hpp"
#include <QStringList>

/// SemVer-версия; метаданные сборки не участвуют в сравнении.
struct Version {
    quint64 major = 0;
    quint64 minor = 0;
    quint64 patch = 0;
    QString prerelease;
    QString text;
};

/// Разобрать полную версию major.minor.patch[-pre][+build].
Version parseVersion(const QString &text);

/// Сравнить версии, включая порядок идентификаторов prerelease.
int compareVersions(const Version &left, const Version &right);

/// Проверить Cargo requirement: ^, ~, =, <, <=, >, >=, wildcard, запятые.
bool matchesRequirement(const Version &version, const QString &requirement);
