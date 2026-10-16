#include "config.hpp"
#include "error.hpp"
#include "repository.hpp"
#include "test_repository.hpp"
#include "graph.hpp"
#include "order.hpp"
#include <memory>
#include <QCoreApplication>
#include <QTextStream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    try {
        const auto config = readConfig(app);
        std::unique_ptr<Repository> repository;
        if (config.testMode) repository = std::make_unique<TestRepository>(config.repository);
        else repository = std::make_unique<CargoRepository>(config.repository);
        const auto graph = buildGraph(*repository, config);
        QTextStream output(stdout);
        if (config.order) {
            output << "Порядок загрузки исследованного графа: "
                   << loadOrder(graph).join(" -> ") << Qt::endl;
            return 0;
        }
        for (auto it = graph.edges.begin(); it != graph.edges.end(); ++it) {
            auto children = it.value().values();
            children.sort();
            output << it.key() << ": " << children.join(", ") << Qt::endl;
        }
        return 0;
    } catch (const GraphError &error) {
        QTextStream(stderr) << "Ошибка: " << error.what() << Qt::endl;
        return 1;
    }
}
