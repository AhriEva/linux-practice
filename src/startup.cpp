#include "startup.hpp"
#include <QFile>
#include <QTextStream>

bool runStartup(Shell &shell, const QString &path, const Output &write) {
    if (path.isEmpty()) return true;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        write("Ошибка скрипта: " + path + ": " + file.errorString());
        return false;
    }
    QTextStream input(&file);
    input.setEncoding(QStringConverter::Utf8);
    bool success = true;
    int lineNumber = 0;
    while (!input.atEnd() && !shell.closed()) {
        const auto line = input.readLine();
        ++lineNumber;
        write(shell.cwd() + "$ " + line);
        const auto result = shell.execute(line);
        if (!result.output.isEmpty()) write(result.output);
        if (!result.ok) {
            write(QString("Скрипт %1, строка %2").arg(path).arg(lineNumber));
            success = false;
        }
    }
    return success;
}
