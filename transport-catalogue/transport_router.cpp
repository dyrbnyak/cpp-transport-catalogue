#include "transport_router.h"
#include <cmath>

TransportRouter::TransportRouter(const TransportCatalogue& catalogue,
                                 const RoutingSettings& settings)
    : catalogue_(catalogue)
    , settings_(settings)
    , graph_(catalogue.GetStop().size() * 2)
    //В графе реализована концепция: два узла на остановку
    // 1) жду автобус
    // 2) в пути между остановками
    // Поэтому надо в два раза больше места для узлов
{
    // Строим граф
    BuildGraph();

    // Поскольку используется алгоритм Флойда–Уоршелла,
    // в него передаем граф, и он находит все кратчайшие пути
    router_ = std::make_unique<graph::Router<double>>(graph_);
}

void TransportRouter::SetRouterSettings(const RoutingSettings &settings){
    settings_ = settings;
}

void TransportRouter::BuildGraph() {
    AddWaitEdges();// Добавляем ребра ожидания
    AddTravelEdges(); // Добавляем ребра поездок
}

void TransportRouter::AddWaitEdges() {
    //Объявляем первоначальный индекс вершин, счетчик
    size_t vertex_id = 0;

    for (StopPtr stop : catalogue_.GetStop()) {
        // Собираем узлы остановок
        StopVertexIds ids;

        ids.wait_id = vertex_id++; // 0,2,4,6,8..
        ids.board_id = vertex_id++; // 1,3,5,7...

        //Добавляем остановку в словарь
        //Чтоб мы по остановки узнать информацию об графе
        stop_vertices_[stop] = ids;

        // Из узлов делаем ребра
        graph::Edge<double> wait_edge{
            ids.wait_id,
            ids.board_id,
            static_cast<double>(settings_.bus_wait_time)
        };

        //Добавляем в граф ребро
        graph::EdgeId edge_id = graph_.AddEdge(wait_edge);

        //Заполняем информацию об элементе
        edge_metadata_[edge_id] = {
            .is_wait = true,
            .stop_name = stop->name,
            .bus_name = "",
            .span_count = 0
        };
    }
}

double TransportRouter::ComputeTravelTime(double distance) const {
    // Время = расстояние / скорость
    // Но кроме этого расстояние между остановками - в метрах, а у нас км в ч
    // Надо перевести
    return (distance / (settings_.bus_velocity * 1000.0)) * 60.0;
}



void TransportRouter::AddTravelEdges() {
    //Перебираем каждый существющий автобусный маршрут
    for (BusPtr bus : catalogue_.GetBus()) {
        // Забираем его остановки для построения граней.
        // Берется full_route,
        // там сразу учтен как обычный, так и заколцкованный маршрут,
        // не нужны доп проверки
        const auto& stops = bus->full_route;

        //Не может быть маршрута, где меньше двух остановок
        if (stops.size() < 2) {
            continue;
        }

        //Берем две остановки и считаем между ними сумарное расстоение
        //Добавляем инфу в список

        //Начинаем с остановки from
        for (size_t i = 0; i < stops.size(); ++i) {
            StopPtr from_stop = stops[i];
            double accumulated_distance = 0.0;

        // Берем остановку to, которая следующая, то есть, i + 1
            for (size_t j = i + 1; j < stops.size(); ++j) {
                StopPtr to_stop = stops[j];

                //Аккумулируем расстояние.
                // Идея такая.
                // Допустим остановки идут A-B-C-D-E
                // Нормальному пацану надо знать, а сколько будет из A - C ? а из A - D ?
                // у них же нет прямых путей
                // И тут мы накапливаем расстояние. в рамках маршрута.
                // из А в В = 100м, Из В в С = 50м -> из A в C = 100 + 50 = 150м
                accumulated_distance += catalogue_.GetDistance(
                    stops[j - 1]->name,
                    stops[j]->name
                    );

                double travel_time = ComputeTravelTime(accumulated_distance);
                int span_count = static_cast<int>(j - i);

                //Добавление ребра
                graph::Edge<double> travel_edge{
                    stop_vertices_[from_stop].board_id,
                    stop_vertices_[to_stop].wait_id,
                    travel_time
                };

                graph::EdgeId edge_id = graph_.AddEdge(travel_edge);

                //Добавление информации об грани для вывода
                edge_metadata_[edge_id] = {
                    .is_wait = false,
                    .stop_name = "",
                    .bus_name = bus->name,
                    .span_count = span_count
                };
            }
        }
    }
}

std::optional<RouteInfo> TransportRouter::BuildRoute(StopPtr from_stop, StopPtr to_stop) const {
    // Собираем их id остановки в графе
    size_t start_vertex = stop_vertices_[from_stop].wait_id;
    size_t end_vertex = stop_vertices_[to_stop].wait_id;

    //Сохраняем кратчайший маршрут
    const auto& route = router_->BuildRoute(start_vertex, end_vertex);

    if(!route.has_value()){
        return {};
    }

    // Аккамулируем всю инфу, для дальнейшего вывода
    RouteInfo result;
    result.total_time = route->weight;

    //Имеем построенный маршрут
    //Теперь надо взять информацию по граням и остановкам из словарей
    //И взять оттуда инфу, которую надо вывести
    for (graph::EdgeId edge_id : route->edges) {
        const auto& metadata = edge_metadata_[edge_id];
        const auto& edge = graph_.GetEdge(edge_id);

        if (metadata.is_wait) {
            result.items.push_back({
                .type = TypeItem::WAIT,
                .stop_name = metadata.stop_name,
                .bus_name = "",
                .span_count = 0,
                .time = edge.weight
            });

        } else {
            result.items.push_back({
                .type = TypeItem::BUS,
                .stop_name = "",
                .bus_name = metadata.bus_name,
                .span_count = metadata.span_count,
                .time = edge.weight
            });
        }
    }

    return result;
}
