#pragma once

#include "shell.hpp"
#include <functional>

using Output = std::function<void(const QString &)>;

/// Выполнить скрипт, показать диалог; продолжать после ошибок команд.
bool runStartup(Shell &shell, const QString &path, const Output &write);
