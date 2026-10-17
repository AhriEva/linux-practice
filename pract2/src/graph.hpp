#pragma once
#include "config.hpp"
#include "repository.hpp"

/// Граф направлен от пакета к его зависимостям.
struct Graph {
    QString root;
    QMap<QString, Package> packages;
    QMap<QString, QSet<QString>> edges;
    QMap<QString, int> depth;
    QSet<QString> truncated;
};

/// Обход BFS без рекурсии, с объединением features и контролем глубины.
Graph buildGraph(Repository &repository, const Config &config);
