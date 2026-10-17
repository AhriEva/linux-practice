#include "options.hpp"

Options parseOptions(const QStringList &args, const QString &allowed) {
    Options result;
    bool ended = false;
    for (qsizetype i = 0; i < args.size(); ++i) {
        const auto &arg = args[i];
        if (!ended && arg == "--") {
            ended = true;
        } else if (!ended && arg == "-L" && allowed.contains('L')) {
            bool valid = false;
            if (++i == args.size()) throw ShellError("-L требует глубину");
            result.depth = args[i].toInt(&valid);
            if (!valid || result.depth < 1)
                throw ShellError("глубина должна быть положительным числом");
        } else if (!ended && arg.startsWith('-') && arg.size() > 1) {
            for (const auto ch : arg.mid(1)) {
                if (!allowed.contains(ch) || ch == 'L')
                    throw ShellError("неизвестная опция: -" + QString(ch));
                result.flags.insert(ch);
            }
        } else result.paths << arg;
    }
    return result;
}
