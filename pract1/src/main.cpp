#include "terminal.hpp"
#include "startup.hpp"
#include <QApplication>
#include <QTextStream>
#include <QMessageBox>

int main(int argc, char *argv[]) {
    for (int index = 1; index < argc; ++index)
        if (QString::fromLocal8Bit(argv[index]) == "--headless")
            qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    const auto config = readConfig(app);
    Shell shell(config);
    QString motd;
    try {
        shell.loadVfs();
        motd = shell.motd();
    } catch (const ShellError &error) {
        const auto message = QString::fromUtf8(error.what());
        if (config.headless) QTextStream(stderr) << message << Qt::endl;
        else QMessageBox::critical(nullptr, "Ошибка загрузки VFS", message);
        return 2;
    }
    if (config.headless) {
        QTextStream output(stdout);
        const Output write = [&output](const QString &line) {
            output << line << Qt::endl;
        };
        write(config.dump());
        if (!motd.isEmpty()) write(motd);
        return runStartup(shell, config.startupPath, write) ? 0 : 1;
    }
    Terminal terminal(shell);
    terminal.write(config.dump());
    terminal.write(motd);
    runStartup(shell, config.startupPath, [&terminal](const QString &line) {
        terminal.write(line);
    });
    if (shell.closed()) return 0;
    terminal.runLine("");
    terminal.show();
    return app.exec();
}
