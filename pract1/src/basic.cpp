#include "basic.hpp"
#include "options.hpp"

namespace {
constexpr unsigned ReadExecute = 5;
constexpr unsigned Execute = 1;

QString listing(const QString &name, const Node &node, bool detailed) {
    const auto label = name + (node.directory ? "/" : "");
    if (!detailed) return label;
    return QString("%1 %2 %3 %4")
        .arg(node.directory ? "d" : "-")
        .arg(node.mode, 3, 8, QChar('0'))
        .arg(node.directory ? 0 : node.data.size()).arg(label);
}

QStringList visibleChildren(const Node &node, const Options &options) {
    QStringList names;
    for (auto it = node.children.cbegin(); it != node.children.cend(); ++it) {
        if (!options.flags.contains('a') && it.key().startsWith('.')) continue;
        if (options.flags.contains('d') && !it.value()->directory) continue;
        names << it.key();
    }
    return names;
}

void walkTree(const Node &node, const Options &options, const QString &prefix,
              int level, QStringList &lines) {
    if (options.depth >= 0 && level >= options.depth) return;
    requireAccess(node, ReadExecute);
    const auto names = visibleChildren(node, options);
    for (qsizetype i = 0; i < names.size(); ++i) {
        const auto child = node.children.value(names[i]);
        const bool last = i == names.size() - 1;
        lines << prefix + (last ? "└── " : "├── ") + names[i];
        if (child->directory)
            walkTree(*child, options, prefix + (last ? "    " : "│   "),
                     level + 1, lines);
    }
}
}

QString listCommand(Vfs &vfs, const QString &cwd, const QStringList &args) {
    const auto options = parseOptions(args, "al");
    if (options.paths.size() > 1) throw ShellError("ls [-al] [путь]");
    const auto path = absolutePath(options.paths.value(0, "."), cwd);
    const auto node = vfs.resolve(path);
    const bool detailed = options.flags.contains('l');
    if (!node->directory)
        return listing(path.section('/', -1), *node, detailed);
    requireAccess(*node, ReadExecute);
    QStringList lines;
    if (options.flags.contains('a')) {
        lines << listing(".", *node, detailed);
        const auto parent = vfs.resolve(absolutePath("..", path));
        lines << listing("..", *parent, detailed);
    }
    for (const auto &name : visibleChildren(*node, options))
        lines << listing(name, *node->children.value(name), detailed);
    return lines.join('\n');
}

QString changeDirectory(Vfs &vfs, QString &cwd, const QStringList &args) {
    if (args.size() > 1) throw ShellError("cd [путь]");
    const auto path = absolutePath(args.value(0, "/"), cwd);
    const auto node = vfs.resolve(path);
    if (!node->directory) throw ShellError("не каталог: " + path);
    requireAccess(*node, Execute);
    cwd = path;
    return {};
}

QString treeCommand(Vfs &vfs, const QString &cwd, const QStringList &args) {
    const auto options = parseOptions(args, "adL");
    if (options.paths.size() > 1) throw ShellError("tree [-ad] [-L N] [путь]");
    const auto path = absolutePath(options.paths.value(0, "."), cwd);
    const auto node = vfs.resolve(path);
    QStringList lines{path};
    if (node->directory) walkTree(*node, options, "", 0, lines);
    return lines.join('\n');
}

QString uniqueCommand(Vfs &vfs, const QString &cwd, const QStringList &args) {
    const auto options = parseOptions(args, "cdu");
    if (options.paths.size() != 1
        || (options.flags.contains('d') && options.flags.contains('u')))
        throw ShellError("uniq [-c] [-d|-u] файл");
    const auto text = vfs.readText(absolutePath(options.paths[0], cwd));
    if (text.isEmpty()) return {};
    auto lines = text.split('\n');
    const bool newline = text.endsWith('\n');
    if (newline) lines.removeLast();
    QStringList output;
    for (qsizetype i = 0; i < lines.size();) {
        auto end = i + 1;
        while (end < lines.size() && lines[end] == lines[i]) ++end;
        const auto count = end - i;
        const bool select = (!options.flags.contains('d') || count > 1)
            && (!options.flags.contains('u') || count == 1);
        if (select) {
            const auto prefix = options.flags.contains('c')
                ? QString::number(count).rightJustified(7) + " " : QString();
            output << prefix + lines[i];
        }
        i = end;
    }
    if (output.isEmpty()) return {};
    return output.join('\n') + (newline ? "\n" : "");
}
