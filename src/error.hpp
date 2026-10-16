#pragma once

#include <QString>
#include <stdexcept>

/// Ошибка команды, конфигурации или структуры виртуальной ФС.
class ShellError : public std::runtime_error {
public:
    explicit ShellError(const QString &message)
        : std::runtime_error(message.toStdString()) {}
};
