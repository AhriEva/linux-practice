#include "shell.hpp"

QStringList parseLine(const QString &line) {
    QStringList words;
    QString word;
    QChar quote;
    bool escaped = false;
    bool started = false;
    for (qsizetype i = 0; i < line.size(); ++i) {
        const QChar ch = line[i];
        if (escaped) {
            word += ch;
            escaped = false;
        } else if (ch == '\\' && quote != '\'') {
            escaped = true;
            started = true;
        } else if (!quote.isNull()) {
            if (ch == quote) quote = QChar();
            else word += ch;
        } else if (ch == '\'' || ch == '"') {
            quote = ch;
            started = true;
        } else if (!started && line.mid(i, 2) == "//") {
            break;
        } else if (ch.isSpace()) {
            if (started) words << word;
            word.clear();
            started = false;
        } else {
            word += ch;
            started = true;
        }
    }
    if (escaped || !quote.isNull())
        throw ShellError("незавершённые кавычки или экранирование");
    if (started) words << word;
    return words;
}

Result Shell::execute(const QString &line) {
    try {
        auto tokens = parseLine(line);
        if (tokens.isEmpty()) return {};
        const auto command = tokens.takeFirst();
        return dispatch(command, tokens);
    } catch (const ShellError &error) {
        return {"Ошибка: " + QString::fromUtf8(error.what()), false};
    }
}

Result Shell::dispatch(const QString &command, const QStringList &args) {
    if (command == "conf-dump") {
        if (!args.isEmpty()) throw ShellError("использование: conf-dump");
        return {config_.dump()};
    }
    if (command == "exit") {
        if (!args.isEmpty()) throw ShellError("использование: exit");
        closed_ = true;
        return {"Сеанс завершён."};
    }
    if (command == "ls" || command == "cd") {
        if (args.size() > 1)
            throw ShellError("использование: " + command + " [путь]");
        return {command + ": [" + args.join(", ") + "]"};
    }
    throw ShellError("неизвестная команда: " + command);
}
