#pragma once

#include "config.hpp"
#include "vfs.hpp"
#include <QStringList>
#include <stdexcept>

/// Разобрать кавычки, экранирование и комментарии //.
QStringList parseLine(const QString &line);

/// Результат команды; ok позволяет сценариям обнаружить ошибку.
struct Result {
    QString output;
    bool ok = true;
};

/// Независимая от GUI сессия оболочки.
class Shell {
public:
    explicit Shell(Config config = {}) : config_(config) {}
    void loadVfs() { vfs_.load(config_.vfsPath); }
    QString motd() const { return vfs_.motd(); }
    Result execute(const QString &line);
    QString cwd() const { return cwd_; }
    bool closed() const { return closed_; }
private:
    Result dispatch(const QString &command, const QStringList &args);
    Config config_;
    Vfs vfs_;
    QString cwd_ = "/";
    bool closed_ = false;
};
