#include "terminal.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Shell shell;
    Terminal terminal(shell);
    terminal.show();
    return app.exec();
}
