#include "identity.hpp"
#include <QtGlobal>
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
