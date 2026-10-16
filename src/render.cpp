#include "render.hpp"

namespace {
QString quoted(QString text) {
    text.replace('\\', "\\\\");
    text.replace('"', "\\\"");
    text.replace('\n', "\\n");
    return '"' + text + '"';
}

QStringList childrenOf(const Graph &graph, const QString &id) {
    auto children = graph.edges.value(id).values();
    children.sort();
    return children;
}

struct TreeFrame {
    QString id;
    QString prefix;
    bool last = true;
    bool root = false;
    QSet<QString> ancestors;
};
}

QString plantUml(const Graph &graph) {
    QStringList lines{"@startuml", "left to right direction", "skinparam componentStyle rectangle"};
    QMap<QString, QString> aliases;
    for (auto it = graph.packages.begin(); it != graph.packages.end(); ++it) {
        const auto alias = "n" + QString::number(aliases.size());
        aliases[it.key()] = alias;
        const auto stereotype = graph.truncated.contains(it.key()) ? " <<depth_limit>>" : "";
        const auto color = it.key() == graph.root ? " #DCEBFF" : "";
        lines << "component " + quoted(it.key()) + " as " + alias + stereotype + color;
    }
    for (auto it = graph.edges.begin(); it != graph.edges.end(); ++it)
        for (const auto &child : childrenOf(graph, it.key()))
            lines << aliases.value(it.key()) + " --> " + aliases.value(child);
    lines << "@enduml";
    return lines.join('\n');
}

QString asciiTree(const Graph &graph) {
    QVector<TreeFrame> stack{{graph.root, {}, true, true, {}}};
    QSet<QString> visited;
    QStringList lines;
    while (!stack.isEmpty()) {
        auto frame = stack.takeLast();
        const bool cycle = frame.ancestors.contains(frame.id);
        const bool shared = visited.contains(frame.id);
        auto label = frame.id;
        if (cycle) label += " [cycle]";
        else if (shared) label += " (*)";
        else if (graph.truncated.contains(frame.id)) label += " [depth limit]";
        const auto branch = frame.root ? "" : (frame.last ? "`-- " : "|-- ");
        lines << frame.prefix + branch + label;
        if (cycle || shared) continue;
        visited.insert(frame.id);
        frame.ancestors.insert(frame.id);
        const auto children = childrenOf(graph, frame.id);
        const auto prefix = frame.prefix + (frame.root ? "" : (frame.last ? "    " : "|   "));
        for (qsizetype i = children.size(); i > 0; --i)
            stack << TreeFrame{children[i - 1], prefix, i == children.size(), false, frame.ancestors};
    }
    return lines.join('\n');
}
