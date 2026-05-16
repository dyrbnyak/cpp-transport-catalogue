#pragma once

#include "graph.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace graph {

// Класс для поиска кратчайших путей между всеми парами вершин
template <typename Weight>
class Router {
private:
    using Graph = DirectedWeightedGraph<Weight>;

public:
    // Конструктор: сразу вычисляет все кратчайшие пути в графе
    // Выбросит исключение, если есть ребро с отрицательным весом
    explicit Router(const Graph& graph);

    // Результат поиска маршрута
    struct RouteInfo {
        Weight weight;               // Суммарный вес пути
        std::vector<EdgeId> edges;   // Список рёбер в порядке от from к to
    };

    // Возвращает кратчайший маршрут между вершинами from и to
    // Если пути нет - возвращает nullopt
    std::optional<RouteInfo> BuildRoute(VertexId from, VertexId to) const;

private:
    // Внутреннее представление пути для алгоритма
    struct RouteInternalData {
        Weight weight;                      // Вес пути
        std::optional<EdgeId> prev_edge;   // Последнее ребро в пути (пусто, если путь нулевой длины)
    };

    // Матрица кратчайших путей: [откуда][куда]
    // Если пути нет - хранится nullopt
    using RoutesInternalData = std::vector<std::vector<std::optional<RouteInternalData>>>;

    // Шаг 1 инициализации алгоритма
    // Заполняет прямые рёбра (пути длины 1) и нулевые пути (вершина -> себя)
    // Если есть несколько рёбер между одними вершинами - оставляет минимальное
    void InitializeRoutesInternalData(const Graph& graph) {
        const size_t vertex_count = graph.GetVertexCount();

        // Обрабатываем каждую вершину как источник
        for (VertexId vertex = 0; vertex < vertex_count; ++vertex) {
            // Путь из вершины в саму себя: вес 0, нет рёбер
            routes_internal_data_[vertex][vertex] = RouteInternalData{ZERO_WEIGHT, std::nullopt};

            // Смотрим на все исходящие рёбра из текущей вершины
            for (const EdgeId edge_id : graph.GetIncidentEdges(vertex)) {
                const auto& edge = graph.GetEdge(edge_id);

                // Проверяем, что вес неотрицательный (требование)
                if (edge.weight < ZERO_WEIGHT) {
                    throw std::domain_error("Edges' weights should be non-negative");
                }

                auto& route_internal_data = routes_internal_data_[vertex][edge.to];

                // Если уже есть путь, но новое ребро лучше - обновляем
                if (!route_internal_data || route_internal_data->weight > edge.weight) {
                    route_internal_data = RouteInternalData{edge.weight, edge_id};
                }
            }
        }
    }

    // Операция релаксации: проверяем, можно ли улучшить путь через промежуточную вершину
    // Пытаемся пройти: from -> (через -> to) и сравниваем с текущим путём from -> to
    void RelaxRoute(VertexId vertex_from, VertexId vertex_to,
                    const RouteInternalData& route_from,
                    const RouteInternalData& route_to) {
        auto& route_relaxing = routes_internal_data_[vertex_from][vertex_to];

        // Вес пути через промежуточную вершину
        const Weight candidate_weight = route_from.weight + route_to.weight;

        // Если новый путь короче (или его ещё нет) - обновляем
        if (!route_relaxing || candidate_weight < route_relaxing->weight) {
            // Определяем последнее ребро в новом пути:
            // - Если в route_to есть prev_edge - значит, путь через промежуточную вершину непустой,
            //   и последнее ребро ведёт в vertex_to
            // - Если route_to.prev_edge пуст - значит, мы "стоим" на промежуточной вершине,
            //   и последнее ребро берём из route_from.prev_edge
            route_relaxing = {candidate_weight,
                              route_to.prev_edge ? route_to.prev_edge : route_from.prev_edge};
        }
    }

    // Шаг 2: релаксация всех путей через конкретную промежуточную вершину
    // Для каждой пары (from, to) смотрим, не короче ли путь from -> through -> to
    void RelaxRoutesInternalDataThroughVertex(size_t vertex_count, VertexId vertex_through) {
        // Перебираем все возможные начальные вершины
        for (VertexId vertex_from = 0; vertex_from < vertex_count; ++vertex_from) {
            // Если есть путь от from до through
            if (const auto& route_from = routes_internal_data_[vertex_from][vertex_through]) {
                // Перебираем все возможные конечные вершины
                for (VertexId vertex_to = 0; vertex_to < vertex_count; ++vertex_to) {
                    // Если есть путь от through до to
                    if (const auto& route_to = routes_internal_data_[vertex_through][vertex_to]) {
                        // Пытаемся улучшить путь от from до to через through
                        RelaxRoute(vertex_from, vertex_to, *route_from, *route_to);
                    }
                }
            }
        }
    }

    static constexpr Weight ZERO_WEIGHT{};  // Ноль для инициализации весов
    const Graph& graph_;                     // Ссылка на исходный граф (не владеем)
    RoutesInternalData routes_internal_data_; // Таблица кратчайших путей
};

// Реализация конструктора
// Выполняет алгоритм Флойда-Уоршелла:
// 1. Инициализация прямыми рёбрами
// 2. Последовательная релаксация через каждую вершину
template <typename Weight>
Router<Weight>::Router(const Graph& graph)
    : graph_(graph)
    // Создаём квадратную матрицу размером vertex_count x vertex_count,
    // заполненную пустыми значениями (nullopt)
    , routes_internal_data_(graph.GetVertexCount(),
                            std::vector<std::optional<RouteInternalData>>(graph.GetVertexCount()))
{
    // Шаг 1: заполняем прямые пути
    InitializeRoutesInternalData(graph);

    // Шаг 2: основной цикл алгоритма
    // Последовательно используем каждую вершину как промежуточную
    const size_t vertex_count = graph.GetVertexCount();
    for (VertexId vertex_through = 0; vertex_through < vertex_count; ++vertex_through) {
        RelaxRoutesInternalDataThroughVertex(vertex_count, vertex_through);
    }
}

// Восстановление маршрута по предвычисленным данным
// Идём с конца к началу, собирая рёбра, затем разворачиваем список
template <typename Weight>
std::optional<typename Router<Weight>::RouteInfo> Router<Weight>::BuildRoute(VertexId from,
                                                                             VertexId to) const {
    // Проверяем, существует ли путь вообще
    const auto& route_internal_data = routes_internal_data_.at(from).at(to);
    if (!route_internal_data) {
        return std::nullopt;  // Путь не найден
    }

    const Weight weight = route_internal_data->weight;
    std::vector<EdgeId> edges;

    // Восстановление пути: начинаем с последнего ребра и идём назад
    // edges будет собран в обратном порядке (от конечной вершины к начальной)
    for (std::optional<EdgeId> edge_id = route_internal_data->prev_edge;
         edge_id;  // Пока есть ребро
         // Переходим к предыдущему ребру: смотрим, откуда пришли в начало текущего ребра
         edge_id = routes_internal_data_[from][graph_.GetEdge(*edge_id).from]->prev_edge)
    {
        edges.push_back(*edge_id);
    }

    // Так как собирали с конца, нужно развернуть
    std::reverse(edges.begin(), edges.end());

    return RouteInfo{weight, std::move(edges)};
}

}  // namespace graph
