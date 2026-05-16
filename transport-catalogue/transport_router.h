// transport_router.h
#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include "domain.h"

#include <unordered_map>
#include <string>
#include <optional>
#include <memory>

class TransportRouter {
public:
    TransportRouter(const TransportCatalogue& catalogue, const RoutingSettings& settings);

    std::optional<RouteInfo> BuildRoute(StopPtr from_stop
                                        ,StopPtr to_stop) const;

    void SetRouterSettings(const RoutingSettings& settings);

private:
    void BuildGraph(); //Построить граф. Сроим один раз при инициализации
    void AddWaitEdges(); // Добавить ребро ожидания
    void AddTravelEdges(); // Добавить ребро автобусного маршрута
    double ComputeTravelTime(double distance) const; // Вычисления времени в пути между остановками


    const TransportCatalogue& catalogue_;
    RoutingSettings settings_;

    // Граф и роутер
    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_;

    // Отображение остановки -> ID её вершин
    mutable std::unordered_map<StopPtr, StopVertexIds> stop_vertices_;

    // Метаданные для каждого ребра (по ID ребра)
    mutable std::unordered_map<graph::EdgeId, EdgeMetadata> edge_metadata_;
};
