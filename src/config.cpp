#include "config.hpp"
#include "error.hpp"
#include "version.hpp"
#include <QFileInfo>
#include <QRegularExpression>
#include <QUrl>

QString Config::dump() const {
    return "package=" + package + "\nrepository=" + repository
        + "\ntest-mode=" + (testMode ? "true" : "false")
        + "\nascii=" + (ascii ? "true" : "false")
        + "\nmax-depth=" + QString::number(maxDepth) + "\nexclude=" + exclude
        + "\norder=" + (order ? "true" : "false");
}

void validateConfig(const Config &config) {
    const QRegularExpression realName("^[A-Za-z][A-Za-z0-9_-]{0,63}$");
    const QRegularExpression testName("^[A-Z]+$");
    const auto name = config.package.section('@', 0, 0);
    if (config.package.isEmpty() || config.package.count('@') > 1
        || !(config.testMode ? testName : realName).match(name).hasMatch()
        || (config.testMode && config.package.contains('@'))
        || config.package.endsWith('@'))
        throw GraphError("неверное имя пакета: " + config.package);
    if (config.package.contains('@')) parseVersion(config.package.section('@', 1));
    if (config.maxDepth < 0) throw GraphError("max-depth должен быть >= 0");
    if (config.exclude.contains('\n') || config.exclude.contains('\r'))
        throw GraphError("exclude не должен содержать переводы строк");
    if (!config.exclude.isEmpty() && name.contains(config.exclude))
        throw GraphError("корневой пакет исключён фильтром");
    if (config.testMode) {
        const QFileInfo file(config.repository);
        if (!file.isFile() || !file.isReadable())
            throw GraphError("тестовый репозиторий недоступен: " + config.repository);
    } else {
        auto urlText = config.repository;
        if (urlText.startsWith("sparse+")) urlText.remove(0, 7);
        const QUrl url(urlText);
        if (!url.isValid() || url.host().isEmpty()
            || (url.scheme() != "https" && url.scheme() != "http")
            || !url.query().isEmpty() || !url.fragment().isEmpty())
            throw GraphError("repository должен быть HTTP(S) URL sparse-индекса");
    }
}

Config readConfig(QCoreApplication &app) {
    QCommandLineParser parser;
    parser.setApplicationDescription("Практика 2, вариант 18: зависимости Cargo");
    parser.addHelpOption();
    parser.addOption({"package", "Имя пакета или name@version", "name"});
    parser.addOption({"repository", "URL sparse-индекса или тестовый JSON", "source",
                      "https://index.crates.io/"});
    parser.addOption({"test-mode", "Использовать тестовый граф"});
    parser.addOption({"order", "Показать порядок загрузки зависимостей"});
    parser.addOption({"ascii", "Дополнительно вывести ASCII-дерево"});
    parser.addOption({"max-depth", "Максимальная глубина: корень = 0", "N", "5"});
    parser.addOption({"exclude", "Не учитывать имена с подстрокой", "text"});
    if (!parser.parse(app.arguments())) throw GraphError(parser.errorText());
    if (parser.isSet("help")) parser.showHelp();
    if (!parser.positionalArguments().isEmpty())
        throw GraphError("позиционные аргументы не поддерживаются");
    bool valid = false;
    const auto depth = parser.value("max-depth").toInt(&valid);
    if (!valid) throw GraphError("max-depth должен быть целым числом");
    Config config{parser.value("package"), parser.value("repository"),
                  parser.isSet("test-mode"), parser.isSet("ascii"),
                  depth, parser.value("exclude")};
    config.order = parser.isSet("order");
    validateConfig(config);
    return config;
}
