#pragma once
#include "repository.hpp"

/// Тестовый репозиторий: JSON-объект {"A":["B"],"B":[]}.
class TestRepository : public Repository {
public:
    explicit TestRepository(const QString &filename);
    Package select(const QString &name, const QString &requirement) override;
private:
    QMap<QString, Package> packages_;
};
