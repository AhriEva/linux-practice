#include "config.hpp"

QString Config::dump() const {
    return "vfs=" + vfsPath + "\nstartup=" + startupPath
        + "\nheadless=" + (headless ? "true" : "false");
}

Config readConfig(QCoreApplication &app) {
    QCommandLineParser parser;
    parser.setApplicationDescription("Вариант 18: эмулятор оболочки");
    parser.addHelpOption();
    parser.addOption({"vfs", "XML-файл виртуальной ФС", "path",
                      "examples/full.xml"});
    parser.addOption({"startup", "Стартовый скрипт UTF-8", "path"});
    parser.addOption({"headless", "Выполнить скрипт без окна (для тестов)"});
    parser.process(app);
    if (!parser.positionalArguments().isEmpty()) parser.showHelp(2);
    return {parser.value("vfs"), parser.value("startup"),
            parser.isSet("headless")};
}
