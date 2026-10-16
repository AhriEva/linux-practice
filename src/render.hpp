#pragma once
#include "graph.hpp"

/// Сформировать самостоятельное текстовое описание PlantUML.
QString plantUml(const Graph &graph);

/// Построить ASCII-дерево без рекурсии; отметить циклы и общие зависимости.
QString asciiTree(const Graph &graph);
