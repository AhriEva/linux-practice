#pragma once
#include <QString>
#include <stdexcept>

/// Ошибка конфигурации, репозитория или графа.
class GraphError : public std::runtime_error {
public:
    explicit GraphError(const QString &message)
        : std::runtime_error(message.toStdString()) {}
};
