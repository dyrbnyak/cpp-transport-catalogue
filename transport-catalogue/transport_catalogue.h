#pragma once

#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <optional>

#include "domain.h"


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
    StopPtr FindStop(std::string_view name_stop) const;

    void AddDistance(const std::string& from, const std::string& to, double distance);

    double ComputeFactDistanceRote(const Stops& stops)const;
    double ComputeGeographicDistanceRote(const Stops& stops) const;

    void AddBus(const std::string& name, const std::vector<StopPtr>& route, bool is_roundtrip);
    BusPtr FindBus(std::string_view name_bus) const;

    //Возвращает все маршруты, которые проходят через переданную остановку
    const std::unordered_set<BusPtr>& GetBusByStop(StopPtr stop) const;

    std::vector<StopPtr> GetStop() const;
    std::vector<BusPtr> GetBus() const;

    void SetRoutingSettings(RoutingSettings routing_settings);

    double GetDistance(const std::string& from, const std::string& to) const;
private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, StopPtr> index_stops_;

    std::deque<Bus> bus_;
    std::unordered_map<std::string_view, BusPtr> index_bus_;

    //Храним указатели на маршруты, которые проходят через остановку. {Остановка, указатель{множество уникальных маршрутов}}
    std::unordered_map<std::string_view, std::unordered_set<BusPtr>> stops_on_route_;


    // Храним значения настроек маршрута: скорость движения автобусов и время ожидания
    RoutingSettings routing_settings_;

    //Здесь хранится значение в формате <остановка откуда, остановка куда> = расстояние

    //Почему используем pair<std::string, std::string>, а не StopPtr;
    //На момент добавления информации о расстоянии до остановки, некоторых остановок может не быть
    //У меня НЕ реализован подход с "болванками", поэтому надежнее добавлять строки
    //и в дальнейшем брать из остановок названия и считать длину маршрута.
    std::unordered_map<std::pair<std::string, std::string>, double, PairHasher> distances_;
};
