#include "order.hpp"
#include <set>

QStringList loadOrder(const Graph &graph) {
    QMap<QString, qsizetype> remaining;
    QMap<QString, QSet<QString>> dependents;
    std::set<QString> ready;
    for (auto it = graph.packages.begin(); it != graph.packages.end(); ++it) {
        const auto children = graph.edges.value(it.key());
        remaining[it.key()] = children.size();
        if (children.isEmpty()) ready.insert(it.key());
        for (const auto &child : children) dependents[child].insert(it.key());
    }
    QStringList result;
    while (!ready.empty()) {
        const auto id = *ready.begin();
        ready.erase(ready.begin());
        result << id;
        for (const auto &parent : dependents.value(id)) {
            --remaining[parent];
            if (remaining.value(parent) == 0) ready.insert(parent);
        }
    }
    if (result.size() != graph.packages.size()) {
        QStringList blocked;
        for (auto it = remaining.begin(); it != remaining.end(); ++it)
            if (it.value() > 0) blocked << it.key();
        throw GraphError("цикл зависимостей: полный порядок невозможен; "
                         "неупорядоченные узлы: " + blocked.join(", "));
    }
    return result;
}
