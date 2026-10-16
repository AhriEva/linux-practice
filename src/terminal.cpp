#include "terminal.hpp"

#include <QFontDatabase>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSysInfo>
#include <QTimer>
#include <QVBoxLayout>
#ifdef Q_OS_UNIX
#include <pwd.h>
#include <unistd.h>
#endif

QString currentUser() {
#ifdef Q_OS_UNIX
    if (const auto *user = getpwuid(getuid()))
        return QString::fromLocal8Bit(user->pw_name);
#endif
    return qEnvironmentVariable("USERNAME", "user");
}

Terminal::Terminal(Shell &shell) : shell_(shell) {
    setWindowTitle("Эмулятор - [" + currentUser() + "@"
                   + QSysInfo::machineHostName() + "]");
    resize(900, 600);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    auto *layout = new QVBoxLayout(this);
    output_ = new QPlainTextEdit(this);
    output_->setReadOnly(true);
    output_->setObjectName("transcript");
    layout->addWidget(output_);
    auto *row = new QHBoxLayout;
    prompt_ = new QLabel("/$", this);
    input_ = new QLineEdit(this);
    input_->setObjectName("commandInput");
    row->addWidget(prompt_);
    row->addWidget(input_);
    layout->addLayout(row);
    connect(input_, &QLineEdit::returnPressed, this, [this] {
        const auto line = input_->text();
        input_->clear();
        runLine(line);
    });
    input_->setFocus();
}

void Terminal::write(const QString &text) {
    if (!text.isEmpty()) output_->appendPlainText(text);
}

void Terminal::runLine(const QString &line) {
    write(shell_.cwd() + "$ " + line);
    write(shell_.execute(line).output);
    prompt_->setText(shell_.cwd() + "$");
    if (shell_.closed()) QTimer::singleShot(0, this, &QWidget::close);
}
