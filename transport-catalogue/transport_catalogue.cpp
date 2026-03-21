#include <unordered_set>
#include <iostream>

using namespace std;

#include "transport_catalogue.h"


void TransportCatalogue::AddStop(const string& new_name, const Coordinates& new_coordinates){
    Stop new_stop{new_name, new_coordinates};
    stops_.push_back(std::move(new_stop));
    index_stops_[stops_.back().name] = &stops_.back();
    stops_on_route_[stops_.back().name] = {};
}

StopPtr TransportCatalogue::FindStop(string_view name_stop) const{
    const auto iterator = index_stops_.find(name_stop);
    return iterator != index_stops_.end() ? iterator->second : nullptr;
}


void TransportCatalogue::AddDistance(const std::string& from, const std::string& to, double distance){
    const StopPtr from_stop = FindStop(from);
    const StopPtr to_stop = FindStop(to);

    if (from_stop && to_stop) {
        distances_[{from_stop->name, to_stop->name}] = distance;
    }
}


double TransportCatalogue::GetDistance(const string& from, const string& to) const{
    // Ищем прямое направление
    auto it = distances_.find({from, to});
    if (it != distances_.end()) {
        return it->second;  // возвращаем значение через итератор
    }

    // Ищем обратное направление
    it = distances_.find({to, from});
    if (it != distances_.end()) {
        return it->second;
    }

    // Если не нашли - возвращаем 0.0
    return 0.0;
}


void TransportCatalogue::AddBus(const string& name, const std::vector<StopPtr>& route, bool is_roundtrip){
    /*
     * Сначала проверяем, есть ли проверяемая остановка, только потом добавляем.
     * В алгоритме применения команд применена логика, сначала добавляются остановки, потом маршруты
     */
    Stops stops;
    for (const auto* stop_name : route) {
        if (FindStop(stop_name -> name) != nullptr) {
            stops.push_back(index_stops_[stop_name -> name]);
        }
    }

    /*
     * Почему нельзя умножать на 2, чтоб получить количество остановок обратно:

    Рассмотрим маршрут с 3 остановками: А → Б → В (не кольцевой)

    Правильный полный маршрут: А → Б → В → Б → А
    Расстояния: (А→Б) + (Б→В) + (В→Б) + (Б→А)

    Если просто умножить на 2: (А→Б) + (Б→В) умножить на 2 = 2*(А→Б) + 2*(Б→В)
    Это НЕПРАВИЛЬНО, потому что:

    В правильном маршруте нет прямого перехода В→А
    В правильном маршруте есть переход В→Б, который не учитывается при умножении
    */

    // Создаем полный маршрут для не кольцевых маршрутов
    Stops full_route = stops;
    if (!is_roundtrip && stops.size() > 1) {
        // Добавляем остановки в обратном порядке, пропуская первую (она уже есть)
        for (auto it = stops.rbegin()  + 1; it != stops.rend(); ++it) {
            full_route.push_back(*it);
        }
    }

    //Переменные с дополнительной информацией о маршруте
    int32_t route_length       =   ComputeFactDistanceRote(full_route);
    int32_t unique_stops       =   unordered_set<StopPtr>(stops.begin(), stops.end()).size();
    int32_t stops_on_rote      =   full_route.size();
    double curvature            =   (route_length * 1.0)  / ComputeGeographicDistanceRote(full_route);

    bus_.push_back({name,
                    std::move(stops),
                    std::move(full_route),
                    stops_on_rote,
                    unique_stops,
                    route_length,
                    curvature,
                    is_roundtrip});

    //Заполняем список: остановка - автобусы, которые через неё ходят.
    for (const auto& stop : bus_.back().stops) {
        stops_on_route_[stop->name].insert(&bus_.back());
    }

    index_bus_[bus_.back().name] = &bus_.back();
}

BusPtr TransportCatalogue::FindBus(string_view name_bus) const{
    auto iterator = index_bus_.find(name_bus);
    return iterator != index_bus_.end() ? iterator -> second : nullptr;
}

const std::unordered_set<BusPtr>& TransportCatalogue::GetBusByStop(StopPtr stop) const{
    if(const auto it = stops_on_route_.find(stop -> name); it != stops_on_route_.end()){
        return it->second;
    }

    static std::unordered_set<BusPtr> empty{};
    return empty;

}

std::vector<StopPtr> TransportCatalogue::GetStop() const{
    std::vector<StopPtr> result;
    result.reserve(stops_.size());

    for(const auto& stop : stops_){
        result.push_back(const_cast<Stop*>(&stop));
    }

    return result;
}

std::vector<BusPtr> TransportCatalogue::GetBus() const{
    std::vector<BusPtr> result;
    result.reserve(bus_.size());

    for(const auto& bus : bus_){
        result.push_back(const_cast<Bus*>(&bus));
    }

    return result;
}


double TransportCatalogue::ComputeFactDistanceRote(const Stops& stops) const{
    double distance = 0;

    if (stops.size() < 2) {
        return 0;
    }

    for(size_t i = 0; i < stops.size()-1; ++i){
        distance += GetDistance(stops[i]->name, stops[i+1]->name);
    }

    return distance;
}


double TransportCatalogue::ComputeGeographicDistanceRote(const Stops& stops) const{
    double distance = 0;

    for(size_t i = 0; i < stops.size()-1; ++i){
        distance += ComputeDistance(stops[i] -> coordinates,  stops[i+1]->coordinates);
    }

    return distance;
}


