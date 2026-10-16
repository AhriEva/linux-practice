#include "config.hpp"
#include "error.hpp"
#include "graph.hpp"
#include "order.hpp"
#include "render.hpp"
#include "test_repository.hpp"
#include <QCoreApplication>
#include <QTextStream>
#include <memory>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    try {
        const auto config = readConfig(app);
        std::unique_ptr<Repository> repository;
        if (config.testMode) repository = std::make_unique<TestRepository>(config.repository);
        else repository = std::make_unique<CargoRepository>(config.repository);
        const auto graph = buildGraph(*repository, config);
        QTextStream output(stdout);
        output << plantUml(graph) << Qt::endl;
        if (config.ascii) output << "\nASCII tree:\n" << asciiTree(graph) << Qt::endl;
        if (config.order) {
            output << "\nПорядок загрузки исследованного графа:\n";
            if (!graph.truncated.isEmpty()) output << "(достигнут предел глубины)\n";
            output << loadOrder(graph).join(" -> ") << Qt::endl;
        }
        return 0;
    } catch (const GraphError &error) {
        QTextStream(stderr) << "Ошибка: " << error.what() << Qt::endl;
        return 1;
    }
}
