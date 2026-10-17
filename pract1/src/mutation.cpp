#include "mutation.hpp"
#include <QRegularExpression>
#include <QVector>

namespace {
constexpr unsigned ReadWriteExecute = 7;
constexpr unsigned WriteExecute = 3;
constexpr unsigned ReadExecute = 5;
constexpr unsigned Read = 4;
constexpr unsigned Write = 2;
constexpr unsigned Execute = 1;
constexpr unsigned UserShift = 6;
constexpr unsigned GroupShift = 3;
constexpr unsigned DefaultDirectoryMode = 0755;

struct MkdirOptions {
    bool parents = false;
    unsigned mode = DefaultDirectoryMode;
    QStringList paths;
};

MkdirOptions mkdirOptions(const QStringList &args) {
    MkdirOptions options;
    bool ended = false;
    for (qsizetype i = 0; i < args.size(); ++i) {
        if (!ended && args[i] == "--") ended = true;
        else if (!ended && args[i] == "-p") options.parents = true;
        else if (!ended && args[i] == "-m") {
            if (++i == args.size()) throw ShellError("-m требует маску");
            options.mode = numericMode(args[i]);
        } else if (!ended && args[i].startsWith('-')) {
            throw ShellError("неизвестная опция: " + args[i]);
        } else options.paths << args[i];
    }
    if (options.paths.isEmpty())
        throw ShellError("mkdir [-p] [-m MODE] путь...");
    return options;
}

void makeDirectory(Vfs &vfs, const QString &path, const MkdirOptions &options) {
    const auto parts = path.split('/', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        if (!options.parents) throw ShellError("каталог уже существует: /");
        return;
    }
    auto current = vfs.resolve("/");
    for (qsizetype i = 0; i < parts.size(); ++i) {
        if (!current->directory) throw ShellError("не каталог: " + path);
        requireAccess(*current, Execute);
        const bool last = i == parts.size() - 1;
        const auto name = parts[i];
        if (current->children.contains(name)) {
            current = current->children.value(name);
            if (!current->directory) throw ShellError("не каталог: " + path);
            if (last && !options.parents)
                throw ShellError("каталог уже существует: " + path);
        } else {
            if (!last && !options.parents)
                throw ShellError("родительский каталог не найден: " + path);
            requireAccess(*current, WriteExecute);
            auto child = std::make_shared<Node>();
            child->mode = last ? options.mode : DefaultDirectoryMode;
            current->children.insert(name, child);
            current = child;
        }
    }
}

unsigned classMask(const QString &who) {
    unsigned mask = 0;
    if (who.isEmpty() || who.contains('a')) return 0777;
    if (who.contains('u')) mask |= ReadWriteExecute << UserShift;
    if (who.contains('g')) mask |= ReadWriteExecute << GroupShift;
    if (who.contains('o')) mask |= ReadWriteExecute;
    return mask;
}

unsigned symbolicMode(unsigned oldMode, const QString &specification) {
    static const QRegularExpression pattern("^([ugoa]*)([+=-])([rwx]*)$");
    for (const auto &clause : specification.split(',')) {
        const auto match = pattern.match(clause);
        if (!match.hasMatch()) throw ShellError("неверная маска: " + clause);
        const auto mask = classMask(match.captured(1));
        const auto rights = match.captured(3);
        unsigned bits = 0;
        if (rights.contains('r')) bits |= Read;
        if (rights.contains('w')) bits |= Write;
        if (rights.contains('x')) bits |= Execute;
        bits = (bits | (bits << GroupShift) | (bits << UserShift)) & mask;
        const auto operation = match.captured(2);
        if (operation == "=") oldMode = (oldMode & ~mask) | bits;
        else if (operation == "+") oldMode |= bits;
        else oldMode &= ~bits;
    }
    return oldMode;
}

unsigned changedMode(unsigned oldMode, const QString &specification) {
    if (!specification.isEmpty() && specification.front().isDigit())
        return numericMode(specification);
    return symbolicMode(oldMode, specification);
}

void collectNodes(const std::shared_ptr<Node> &node, bool recursive,
                  QVector<std::shared_ptr<Node>> &nodes) {
    nodes << node;
    if (!recursive || !node->directory) return;
    requireAccess(*node, ReadExecute);
    for (const auto &child : node->children) collectNodes(child, true, nodes);
}
}

QString mkdirCommand(Vfs &vfs, const QString &cwd, const QStringList &args) {
    const auto options = mkdirOptions(args);
    for (const auto &path : options.paths)
        makeDirectory(vfs, absolutePath(path, cwd), options);
    return {};
}

QString chmodCommand(Vfs &vfs, const QString &cwd, const QStringList &args) {
    auto operands = args;
    bool recursive = false;
    if (!operands.isEmpty() && operands.first() == "-R") {
        recursive = true;
        operands.removeFirst();
    }
    if (!operands.isEmpty() && operands.first() == "--") operands.removeFirst();
    if (operands.size() < 2) throw ShellError("chmod [-R] MODE путь...");
    const auto specification = operands.takeFirst();
    QVector<std::shared_ptr<Node>> nodes;
    for (const auto &path : operands)
        collectNodes(vfs.resolve(absolutePath(path, cwd)), recursive, nodes);
    QVector<unsigned> modes;
    for (const auto &node : nodes)
        modes << changedMode(node->mode, specification);
    for (qsizetype i = 0; i < nodes.size(); ++i) nodes[i]->mode = modes[i];
    return {};
}
