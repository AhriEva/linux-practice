#include "version.hpp"
#include <QRegularExpression>

namespace {
int compareNumber(quint64 left, quint64 right) {
    return left < right ? -1 : (left > right ? 1 : 0);
}

bool isNumeric(const QString &text) {
    static const QRegularExpression digits("^[0-9]+$");
    return digits.match(text).hasMatch();
}

void validateIdentifiers(const QString &text, bool prerelease) {
    if (text.isEmpty()) return;
    for (const auto &part : text.split('.')) {
        if (part.isEmpty()) throw GraphError("пустой идентификатор SemVer");
        if (prerelease && isNumeric(part) && part.size() > 1
            && part.startsWith('0'))
            throw GraphError("ведущий ноль в prerelease");
    }
}

int comparePrerelease(const QString &left, const QString &right) {
    if (left == right) return 0;
    if (left.isEmpty()) return 1;
    if (right.isEmpty()) return -1;
    const auto a = left.split('.');
    const auto b = right.split('.');
    for (qsizetype i = 0; i < qMin(a.size(), b.size()); ++i) {
        if (a[i] == b[i]) continue;
        const bool an = isNumeric(a[i]);
        const bool bn = isNumeric(b[i]);
        if (an != bn) return an ? -1 : 1;
        if (an && a[i].size() != b[i].size())
            return a[i].size() < b[i].size() ? -1 : 1;
        return a[i] < b[i] ? -1 : 1;
    }
    return a.size() < b.size() ? -1 : 1;
}

struct Clause {
    QString operation;
    Version base;
    int precision = 0;
    bool wildcard = false;
};

Clause parseClause(QString text) {
    static const QRegularExpression prefix("^(>=|<=|>|<|=|\\^|~)?\\s*(\\S+)$");
    const auto match = prefix.match(text.trimmed());
    if (!match.hasMatch()) throw GraphError("неверное требование версии: " + text);
    Clause clause;
    clause.operation = match.captured(1);
    auto value = match.captured(2);
    const auto suffixAt = value.indexOf(QRegularExpression("[-+]"));
    const auto numeric = suffixAt < 0 ? value : value.left(suffixAt);
    auto parts = numeric.split('.');
    if (parts.size() > 3) throw GraphError("неверная версия: " + value);
    for (const auto &part : parts) {
        if (part == "*" || part == "x" || part == "X") clause.wildcard = true;
        else {
            if (clause.wildcard || !isNumeric(part))
                throw GraphError("неверный wildcard: " + value);
            ++clause.precision;
        }
    }
    parts = parts.mid(0, clause.precision);
    while (parts.size() < 3) parts << "0";
    if (suffixAt >= 0) {
        if (clause.precision != 3 || clause.wildcard)
            throw GraphError("prerelease требует полную версию");
        clause.base = parseVersion(value);
    } else clause.base = parseVersion(parts.join('.'));
    if (clause.wildcard && !clause.operation.isEmpty()
        && clause.operation != "=")
        throw GraphError("wildcard нельзя сочетать с этим оператором");
    return clause;
}

Version upperBound(const Version &base, int component) {
    Version upper = base;
    upper.prerelease.clear();
    if (component == 0) { ++upper.major; upper.minor = 0; upper.patch = 0; }
    else if (component == 1) { ++upper.minor; upper.patch = 0; }
    else ++upper.patch;
    if ((component == 0 && upper.major == 0)
        || (component == 1 && upper.minor == 0)
        || (component == 2 && upper.patch == 0))
        throw GraphError("переполнение границы версии");
    return upper;
}

bool matchClause(const Version &version, const Clause &clause) {
    if (clause.precision == 0) return true;
    const int comparison = compareVersions(version, clause.base);
    const auto op = clause.operation;
    const bool partial = clause.precision < 3;
    if (op == ">=") return comparison >= 0;
    if (op == "<") return comparison < 0;
    if (op == ">") return partial
        ? compareVersions(version, upperBound(clause.base, clause.precision - 1)) >= 0
        : comparison > 0;
    if (op == "<=") return partial
        ? compareVersions(version, upperBound(clause.base, clause.precision - 1)) < 0
        : comparison <= 0;
    if (op == "=" && !partial && !clause.wildcard) return comparison == 0;
    int component = 0;
    if (clause.wildcard || op == "=") component = clause.precision - 1;
    else if (op == "~") component = clause.precision > 1 ? 1 : 0;
    else {
        if (clause.base.major != 0) component = 0;
        else if (clause.base.minor != 0 || clause.precision == 2) component = 1;
        else component = clause.precision == 1 ? 0 : 2;
    }
    return comparison >= 0
        && compareVersions(version, upperBound(clause.base, component)) < 0;
}
}

Version parseVersion(const QString &text) {
    static const QRegularExpression pattern(
        "^(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)"
        "(?:-([0-9A-Za-z.-]+))?(?:\\+([0-9A-Za-z.-]+))?$");
    const auto match = pattern.match(text);
    if (!match.hasMatch()) throw GraphError("неверная SemVer-версия: " + text);
    Version result;
    quint64 *numbers[] = {&result.major, &result.minor, &result.patch};
    for (int i = 0; i < 3; ++i) {
        bool valid = false;
        *numbers[i] = match.captured(i + 1).toULongLong(&valid);
        if (!valid) throw GraphError("слишком большое число SemVer");
    }
    result.prerelease = match.captured(4);
    result.text = text;
    validateIdentifiers(result.prerelease, true);
    validateIdentifiers(match.captured(5), false);
    return result;
}

int compareVersions(const Version &left, const Version &right) {
    const int major = compareNumber(left.major, right.major);
    if (major) return major;
    const int minor = compareNumber(left.minor, right.minor);
    if (minor) return minor;
    const int patch = compareNumber(left.patch, right.patch);
    return patch ? patch : comparePrerelease(left.prerelease, right.prerelease);
}

bool matchesRequirement(const Version &version, const QString &requirement) {
    if (requirement.trimmed().isEmpty()) throw GraphError("пустое требование версии");
    bool prereleaseAllowed = version.prerelease.isEmpty();
    bool matches = true;
    for (const auto &text : requirement.split(',')) {
        const auto clause = parseClause(text);
        const auto &base = clause.base;
        if (!base.prerelease.isEmpty() && version.major == base.major
            && version.minor == base.minor && version.patch == base.patch)
            prereleaseAllowed = true;
        matches = matchClause(version, clause) && matches;
    }
    return matches && prereleaseAllowed;
}
