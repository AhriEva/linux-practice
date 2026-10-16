#pragma once

#include "error.hpp"
#include <QMap>
#include <QStringList>
#include <memory>

/// Узел VFS; содержимое и права существуют только в оперативной памяти.
struct Node {
    bool directory = true;
    unsigned mode = 0755;
    QByteArray data;
    QMap<QString, std::shared_ptr<Node>> children;
};

/// Канонический абсолютный путь; переход выше / остаётся в /.
QString absolutePath(const QString &path, const QString &cwd);

/// Проверить права владельца виртуального узла (r=4,w=2,x=1).
void requireAccess(const Node &node, unsigned bits);

/// Разобрать восьмеричную маску 000..777; специальные биты не поддерживаются.
unsigned numericMode(const QString &text);

/// Загрузчик XML и операции с деревом без записи на диск.
class Vfs {
public:
    Vfs();
    void load(const QString &filename);
    std::shared_ptr<Node> resolve(const QString &path) const;
    QString readText(const QString &path) const;
    QString motd() const;
private:
    std::shared_ptr<Node> root_;
};
