#include "package.hpp"
#include <QQueue>

QVector<ActiveDependency> activeDependencies(const Package &package,
                                            const FeatureRequest &request) {
    QSet<QString> enabled;
    QMap<QString, QSet<QString>> childFeatures;
    QSet<QString> seen;
    QQueue<QString> queue;
    for (const auto &name : request.names) queue.enqueue(name);
    if (request.defaults && package.features.contains("default")) queue.enqueue("default");
    for (const auto &dep : package.dependencies)
        if (!dep.optional) enabled.insert(dep.alias);
    while (!queue.isEmpty()) {
        const auto feature = queue.dequeue();
        if (seen.contains(feature)) continue;
        seen.insert(feature);
        if (feature.startsWith("dep:")) enabled.insert(feature.mid(4));
        else if (feature.contains('/')) {
            auto alias = feature.section('/', 0, 0);
            if (!alias.endsWith('?')) enabled.insert(alias);
            alias.remove('?');
            childFeatures[alias].insert(feature.section('/', 1));
        } else if (package.features.contains(feature)) {
            for (const auto &child : package.features.value(feature)) queue.enqueue(child);
        } else {
            bool optional = false;
            for (const auto &dep : package.dependencies)
                if (dep.alias == feature && dep.optional) optional = true;
            if (!optional) throw GraphError("неизвестная feature " + feature + " у " + package.id());
            enabled.insert(feature);
        }
    }
    QVector<ActiveDependency> result;
    for (const auto &dep : package.dependencies) {
        if (dep.kind == "dev" || (dep.optional && !enabled.contains(dep.alias))) continue;
        FeatureRequest child{dep.defaultFeatures, childFeatures.value(dep.alias)};
        for (const auto &feature : dep.features) child.names.insert(feature);
        result << ActiveDependency{dep, child};
    }
    return result;
}
