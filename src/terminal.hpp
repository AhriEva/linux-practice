#pragma once

#include "shell.hpp"
#include <QWidget>

class QLineEdit;
class QPlainTextEdit;
class QLabel;

/// Графический REPL с неизменяемым журналом ввода и вывода.
class Terminal : public QWidget {
public:
    explicit Terminal(Shell &shell);
    void write(const QString &text);
    void runLine(const QString &line);
private:
    Shell &shell_;
    QPlainTextEdit *output_;
    QLineEdit *input_;
    QLabel *prompt_;
};

/// Получить имя реального пользователя ОС.
QString currentUser();
