#pragma once

#include "error.hpp"
#include <QSet>
#include <QStringList>

/// Результат разбора коротких опций и путей.
struct Options {
    QSet<QChar> flags;
    QStringList paths;
    int depth = -1;
};

/// Разобрать объединённые флаги, -- и необязательный -L N.
Options parseOptions(const QStringList &args, const QString &allowed);
