#pragma once

#include "config.hpp"
#include <QStringList>
#include <stdexcept>

/// Ошибка пользовательской команды или структуры VFS.
class ShellError : public std::runtime_error {
public:
    explicit ShellError(const QString &message);
};

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
    Result execute(const QString &line);
    QString cwd() const { return cwd_; }
    bool closed() const { return closed_; }
private:
    Result dispatch(const QString &command, const QStringList &args);
    Config config_;
    QString cwd_ = "/";
    bool closed_ = false;
};
