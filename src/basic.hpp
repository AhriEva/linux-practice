#pragma once

#include "vfs.hpp"

/// Список файлов: -a включает скрытые, -l показывает права и размер.
QString listCommand(Vfs &vfs, const QString &cwd, const QStringList &args);

/// Перейти в каталог; без аргументов перейти в корень VFS.
QString changeDirectory(Vfs &vfs, QString &cwd, const QStringList &args);

/// Дерево каталогов: -a, -d (только каталоги), -L глубина.
QString treeCommand(Vfs &vfs, const QString &cwd, const QStringList &args);

/// Убрать только соседние повторы строк; -c, -d, -u.
QString uniqueCommand(Vfs &vfs, const QString &cwd, const QStringList &args);
