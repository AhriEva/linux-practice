#include "config.hpp"
#include "error.hpp"
#include <QCoreApplication>
#include <QTextStream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    try {
        const auto config = readConfig(app);
        QTextStream(stdout) << config.dump() << Qt::endl;
        return 0;
    } catch (const GraphError &error) {
        QTextStream(stderr) << "Ошибка: " << error.what() << Qt::endl;
        return 1;
    }
}
