#pragma once
#include "geo.h"
#include <vector>
#include <cstdint>
#include <set>

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например, Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
struct Stop;
struct Bus;
struct CompareStopByName;

using StopPtr = Stop*;
using BusPtr = Bus*;

using Stops = std::vector<StopPtr>;
using Buses = std::vector<BusPtr>;

using UniqueStops = std::set<StopPtr, CompareStopByName>;


struct RouteNames{
    std::vector<std::string> bus_names;
    std::vector<std::string> stop_names;
};

struct RoutePtr{
    std::vector<BusPtr> bus_ptr;
    std::vector<StopPtr> stop_ptr;
};

struct Stop{
    std::string name;
    Coordinates coordinates;
};

struct Bus{
    std::string name;
    Stops stops;
    Stops full_route;
    int32_t stops_on_rote;
    int32_t unique_stops;
    int32_t route_length;
    double curvature;
    bool is_roundtrip = false;
};

struct BusStat{
    int32_t stops_on_rote;
    int32_t unique_stops;
    int32_t route_length;
    double curvature;
};


//Функтор, чтоб задать правло сортировки при добавлении элемента в set
struct CompareStopByName {
    bool operator()(const StopPtr lhs, const StopPtr rhs) const {
        return lhs->name < rhs->name;
    }
};

inline UniqueStops GetUiqueStops(const Buses& buses){
    UniqueStops result;

    for (const auto* bus : buses) {
        for (auto* stop : bus->stops) {
            result.insert((stop));
        }
    }

   return result;
}

inline std::vector<Coordinates> GetCoordinatesFromBuses(const Buses& buses) {
    std::vector<Coordinates> result;

    for (const auto& bus : buses){
        for (const auto& stop : bus->stops){
            result.push_back(stop->coordinates);
        }
    }

   return result;
}
