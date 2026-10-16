#include "vfs.hpp"

#include <QFile>
#include <QRegularExpression>
#include <QStringConverter>
#include <QXmlStreamReader>

namespace {
constexpr unsigned OwnerShift = 6;
constexpr unsigned ReadBit = 4;
constexpr unsigned ExecuteBit = 1;
constexpr unsigned AccessMask = 7;
constexpr qsizetype MaxDepth = 128;

void validateName(const QString &name) {
    if (name.isEmpty() || name == "." || name == ".."
        || name.contains('/') || name.contains(QChar::Null))
        throw ShellError("недопустимое имя узла XML: " + name);
}

std::shared_ptr<Node> readNode(QXmlStreamReader &xml, qsizetype depth) {
    if (depth > MaxDepth) throw ShellError("слишком глубокое дерево XML");
    const auto tag = xml.name().toString();
    if (tag != "dir" && tag != "file")
        throw ShellError("неизвестный элемент XML: " + tag);
    auto node = std::make_shared<Node>();
    node->directory = tag == "dir";
    const auto attrs = xml.attributes();
    node->mode = node->directory ? 0755 : 0644;
    if (attrs.hasAttribute("mode"))
        node->mode = numericMode(attrs.value("mode").toString());
    if (node->directory) {
        while (xml.readNextStartElement()) {
            const auto name = xml.attributes().value("name").toString();
            validateName(name);
            if (node->children.contains(name))
                throw ShellError("повтор имени XML: " + name);
            node->children.insert(name, readNode(xml, depth + 1));
        }
        return node;
    }
    const auto encoding = attrs.value("encoding").toString();
    const auto data = xml.readElementText().toUtf8();
    if (encoding.isEmpty() || encoding == "text") node->data = data;
    else if (encoding == "base64") {
        auto compact = data;
        compact.replace("\n", "").replace("\r", "");
        compact.replace(" ", "").replace("\t", "");
        const auto decoded = QByteArray::fromBase64Encoding(
            compact, QByteArray::AbortOnBase64DecodingErrors);
        if (!decoded) throw ShellError("некорректный base64 в XML");
        node->data = decoded.decoded;
    } else throw ShellError("неизвестная кодировка XML: " + encoding);
    return node;
}
}

unsigned numericMode(const QString &text) {
    static const QRegularExpression pattern("^[0-7]{3,4}$");
    bool valid = false;
    const auto mode = text.toUInt(&valid, 8);
    if (!pattern.match(text).hasMatch() || !valid || mode > 0777)
        throw ShellError("права должны быть в диапазоне 000..777: " + text);
    return mode;
}

QString absolutePath(const QString &path, const QString &cwd) {
    if (path.isEmpty()) throw ShellError("пустой путь");
    QStringList result;
    const auto full = path.startsWith('/') ? path : cwd + "/" + path;
    for (const auto &part : full.split('/', Qt::SkipEmptyParts)) {
        if (part == ".") continue;
        if (part == "..") {
            if (!result.isEmpty()) result.removeLast();
        } else result << part;
    }
    return "/" + result.join('/');
}

void requireAccess(const Node &node, unsigned bits) {
    const auto owner = (node.mode >> OwnerShift) & AccessMask;
    if ((owner & bits) != bits) throw ShellError("доступ запрещён");
}

Vfs::Vfs() : root_(std::make_shared<Node>()) {}

void Vfs::load(const QString &filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        throw ShellError("не удалось открыть VFS: " + file.errorString());
    const auto content = file.readAll();
    if (content.contains("<!DOCTYPE"))
        throw ShellError("DTD в XML не поддерживается");
    QXmlStreamReader xml(content);
    if (!xml.readNextStartElement() || xml.name() != u"vfs")
        throw ShellError("ожидается корень <vfs>");
    auto candidate = std::make_shared<Node>();
    if (xml.attributes().hasAttribute("mode"))
        candidate->mode = numericMode(xml.attributes().value("mode").toString());
    while (xml.readNextStartElement()) {
        const auto name = xml.attributes().value("name").toString();
        validateName(name);
        if (candidate->children.contains(name))
            throw ShellError("повтор имени XML: " + name);
        candidate->children.insert(name, readNode(xml, 1));
    }
    while (!xml.atEnd()) xml.readNext();
    if (xml.hasError())
        throw ShellError("ошибка XML: " + xml.errorString());
    root_ = candidate;
}

std::shared_ptr<Node> Vfs::resolve(const QString &path) const {
    auto current = root_;
    for (const auto &part : path.split('/', Qt::SkipEmptyParts)) {
        if (!current->directory) throw ShellError("не каталог: " + path);
        requireAccess(*current, ExecuteBit);
        if (!current->children.contains(part))
            throw ShellError("путь не найден: " + path);
        current = current->children.value(part);
    }
    return current;
}

QString Vfs::readText(const QString &path) const {
    const auto node = resolve(path);
    if (node->directory) throw ShellError("это каталог: " + path);
    requireAccess(*node, ReadBit);
    QStringDecoder decoder(QStringDecoder::Utf8);
    const QString text = decoder(node->data);
    if (decoder.hasError() || text.contains(QChar::Null))
        throw ShellError("файл не является текстом UTF-8: " + path);
    return text;
}

QString Vfs::motd() const {
    if (!root_->children.contains("motd")) return {};
    return readText("/motd");
}
