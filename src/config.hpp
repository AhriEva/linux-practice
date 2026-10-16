#pragma once
#include <QCommandLineParser>

/// Параметры анализа зависимостей.
struct Config {
    QString package;
    QString repository = "https://index.crates.io/";
    bool testMode = false;
    bool ascii = false;
    int maxDepth = 5;
    QString exclude;
    bool order = false;
    QString dump() const;
};

/// Проверить значения независимо от способа получения параметров.
void validateConfig(const Config &config);

/// Разобрать параметры CLI; --help обрабатывается средствами Qt.
Config readConfig(QCoreApplication &app);
