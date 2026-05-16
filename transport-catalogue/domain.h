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
//Информация об остановке
struct Stop;

//Функтор, чтоб задать правло сортировки при добавлении элемента в set
struct CompareStopByName;

// Информация об именах маршрутов и остановок
struct RouteNames;

// Указатели маршруты и остановки
struct RoutePtr;

//Информаиця об маршруте
struct Bus;

// Информация об маршруте для вывода
struct BusStat;

// Информация об параметрах маршрута
struct RoutingSettings;

// Информация об построенном маршруте
// сумарное время + элементы маршрута(wait + bus)
struct RouteInfo;

// Хранит узел ожидания + узел остановки каж
struct StopVertexIds;

// Типы элемента маршрута
enum class TypeItem{
    WAIT
    ,BUS
};


// Метаданные о ребре для восстановления ответа
struct EdgeMetadata;




//Псевдонимы
using StopPtr = Stop*;
using BusPtr = Bus*;

using Stops = std::vector<StopPtr>;
using Buses = std::vector<BusPtr>;

using UniqueStops = std::set<StopPtr, CompareStopByName>;


//Вспомогательные функции
// Cбор координат остановок на маршруте для визуализации
inline std::vector<Coordinates> GetCoordinatesFromBuses(const Buses& buses);

// Сбор набора уникальных остановок в маршруте
inline UniqueStops GetUiqueStops(const Buses& buses);

struct StopVertexIds {
    size_t wait_id;   // вершина "жду"
    size_t board_id;  // вершина "сажусь"
};

struct RouteItem {
    TypeItem type;              // "Wait" или "Bus"
    std::string stop_name;      // для Wait: на какой остановке ждём
    std::string bus_name;       // для Bus: какой автобус
    int span_count;             // для Bus: сколько перегонов проехали
    double time;                // сколько времени занял этот шаг
};

struct RouteInfo {
    double total_time;              // Суммарное время
    std::vector<RouteItem> items;   // Контейнер с элементами маршрута
};

struct EdgeMetadata {
    bool is_wait;           // true - ребро ожидания, false - ребро поездки
    std::string stop_name;  // для is_wait == true
    std::string bus_name;   // для is_wait == false
    int span_count;         // для is_wait == false (количество перегонов)
};


struct RoutingSettings{
    //время ожидания автобуса на остановке, в минутах. от 1 до 1000
    int bus_wait_time = 1;

    //скорость автобуса, в км/ч. от 1 до 1000
    int bus_velocity = 1;
};

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


