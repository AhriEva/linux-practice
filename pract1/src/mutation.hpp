#pragma once
#include "vfs.hpp"

/// Создать каталоги; -p создаёт родителей, -m MODE задаёт права.
QString mkdirCommand(Vfs &vfs, const QString &cwd, const QStringList &args);

/// Изменить права в памяти: [-R] MODE PATH...; числовые и символьные маски.
QString chmodCommand(Vfs &vfs, const QString &cwd, const QStringList &args);
