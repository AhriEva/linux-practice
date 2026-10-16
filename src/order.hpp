#pragma once
#include "graph.hpp"

/// Топологический порядок: каждая зависимость раньше использующего её пакета.
/// При цикле выбрасывается GraphError; применяется алгоритм Кана без рекурсии.
QStringList loadOrder(const Graph &graph);
