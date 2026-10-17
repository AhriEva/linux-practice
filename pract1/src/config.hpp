#pragma once

#include <QCommandLineParser>

/// Параметры запуска эмулятора.
struct Config {
    QString vfsPath = "examples/full.xml";
    QString startupPath;
    bool headless = false;
    QString dump() const;
};

/// Разобрать параметры; Qt обрабатывает --help и неизвестные опции.
Config readConfig(QCoreApplication &app);
