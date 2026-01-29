#pragma once

#include <deque>
#include <unordered_map>
#include <vector>
#include <string>
#include <set>
#include <optional>

#include "geo.h"


struct Stop{
    std::string name;
    Coordinates coordinates;
};

using Stops = std::vector<Stop*>;

struct Bus{
    std::string name;
    Stops stops;
};

struct BusInfo{
    size_t stops_on_rote;
    size_t unique_stops;
    size_t route_length;
    double curvature;
};

// Хешер для пары указателей на остановки
struct PairHasher {
    size_t operator()(const std::pair<std::string, std::string>& obj) const {
        static constexpr size_t prime = 31;

        size_t h1 = std::hash<std::string>{}(obj.first);
        size_t h2 = std::hash<std::string>{}(obj.second);

        return h1 * prime + h2;
    }
};



class TransportCatalogue {
public:
    void AddStop(const std::string& new_name, const Coordinates& new_coordinates);
    const Stop* FindStop(std::string_view name_stop) const;

    void AddDistance(std::string_view from, std::string_view to, double distance);

    double ComputeFactDistanceRote(const Bus& bus) const;
    double ComputeGeographicDistanceRote(const Bus& bus) const;

    void AddBus(const std::string& name, const std::vector<std::string_view>& route);
    const Bus* FindBus(std::string_view name_bus) const;
    const std::set<std::string>& GetBusByStop(std::string_view name_stop) const;


    BusInfo GetInfo(const Bus* bus) const;
private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> index_stops_;

    std::deque<Bus> bus_;
    std::unordered_map<std::string_view, Bus*> index_bus_;

    std::unordered_map<std::string_view, std::set<std::string>> stops_on_route_;

    //Здесь хранится значение в формате <остановка откуда, остановка куда> = расстояние

    //Почему используем pair<std::string, std::string>, а не StopPtr;
    //На момент добавления информации о расстоянии до остановки, некоторых остановок может не быть
    //У меня не реализован подход с "болванками", поэтому надежнее добавлять строки
    //и в дальнейшем брать из остановок названия и считать длину маршрута.
    std::unordered_map<std::pair<std::string, std::string>, double, PairHasher> distances_;

    double GetDistance(const std::string& from, const std::string& to) const;
};
