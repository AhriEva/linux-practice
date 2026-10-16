#include "config.hpp"
#include "error.hpp"
#include "repository.hpp"
#include <QCoreApplication>
#include <QTextStream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    try {
        const auto config = readConfig(app);
        if (config.testMode) throw GraphError("тестовый граф появится на этапе 3");
        CargoRepository repository(config.repository);
        const auto name = config.package.section('@', 0, 0);
        const auto requirement = config.package.contains('@')
            ? "=" + config.package.section('@', 1) : "*";
        const auto package = repository.select(name, requirement);
        QTextStream output(stdout);
        output << package.id() << Qt::endl;
        for (const auto &dep : activeDependencies(package, {}))
            output << dep.dependency.name << " " << dep.dependency.requirement << Qt::endl;
        return 0;
    } catch (const GraphError &error) {
        QTextStream(stderr) << "Ошибка: " << error.what() << Qt::endl;
        return 1;
    }
}
