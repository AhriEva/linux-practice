#include "graph.hpp"
#include <QQueue>

namespace {
constexpr qsizetype MaximumNodes = 10000;

bool excluded(const QString &name, const Config &config) {
    return !config.exclude.isEmpty() && name.contains(config.exclude);
}

bool mergeRequest(FeatureRequest &target, const FeatureRequest &source) {
    const auto before = target.names;
    const bool defaults = target.defaults;
    target.defaults = target.defaults || source.defaults;
    target.names.unite(source.names);
    return before != target.names || defaults != target.defaults;
}

void addNode(Graph &graph, const Package &package, int depth) {
    if (graph.packages.size() >= MaximumNodes)
        throw GraphError("превышен предел 10000 узлов графа");
    graph.packages.insert(package.id(), package);
    graph.depth.insert(package.id(), depth);
    graph.edges[package.id()];
}

void checkRegistry(const Dependency &dep, const Config &config) {
    if (dep.registry.isEmpty()) return;
    auto source = config.repository;
    if (source.startsWith("sparse+")) source.remove(0, 7);
    while (source.endsWith('/')) source.chop(1);
    const bool cratesIo = source == "https://index.crates.io"
        && dep.registry == "https://github.com/rust-lang/crates.io-index";
    if (!cratesIo && dep.registry != source)
        throw GraphError("зависимость из другого registry не поддержана: " + dep.registry);
}
}

Graph buildGraph(Repository &repository, const Config &config) {
    Graph graph;
    const auto name = config.package.section('@', 0, 0);
    const auto requirement = config.package.contains('@')
        ? "=" + parseVersion(config.package.section('@', 1)).text : "*";
    const auto root = repository.select(name, requirement);
    if (excluded(root.name, config)) throw GraphError("корень исключён фильтром");
    graph.root = root.id();
    addNode(graph, root, 0);
    QMap<QString, FeatureRequest> requests;
    requests[graph.root] = {};
    QQueue<QString> queue;
    QSet<QString> queued;
    queue.enqueue(graph.root);
    queued.insert(graph.root);
    while (!queue.isEmpty()) {
        const auto id = queue.dequeue();
        queued.remove(id);
        const auto dependencies = activeDependencies(graph.packages.value(id), requests.value(id));
        graph.truncated.remove(id);
        for (const auto &active : dependencies) {
            const auto &dep = active.dependency;
            if (excluded(dep.name, config)) continue;
            if (graph.depth.value(id) >= config.maxDepth) {
                graph.truncated.insert(id);
                continue;
            }
            checkRegistry(dep, config);
            const auto child = repository.select(dep.name, dep.requirement);
            if (excluded(child.name, config)) continue;
            const auto childId = child.id();
            const auto depth = graph.depth.value(id) + 1;
            const bool fresh = !graph.packages.contains(childId);
            bool changed = fresh;
            if (fresh) {
                addNode(graph, child, depth);
                requests[childId] = active.request;
            } else {
                changed = mergeRequest(requests[childId], active.request);
                if (depth < graph.depth.value(childId)) {
                    graph.depth[childId] = depth;
                    changed = true;
                }
            }
            graph.edges[id].insert(childId);
            if (changed && !queued.contains(childId)) {
                queue.enqueue(childId);
                queued.insert(childId);
            }
        }
    }
    return graph;
}
