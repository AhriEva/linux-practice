#include "terminal.hpp"
#include "startup.hpp"
#include <QApplication>
#include <QTextStream>

int main(int argc, char *argv[]) {
    for (int index = 1; index < argc; ++index)
        if (QString::fromLocal8Bit(argv[index]) == "--headless")
            qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    const auto config = readConfig(app);
    Shell shell(config);
    if (config.headless) {
        QTextStream output(stdout);
        const Output write = [&output](const QString &line) {
            output << line << Qt::endl;
        };
        write(config.dump());
        return runStartup(shell, config.startupPath, write) ? 0 : 1;
    }
    Terminal terminal(shell);
    terminal.write(config.dump());
    runStartup(shell, config.startupPath, [&terminal](const QString &line) {
        terminal.write(line);
    });
    if (shell.closed()) return 0;
    terminal.runLine("");
    terminal.show();
    return app.exec();
}
