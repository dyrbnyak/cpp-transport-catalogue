#include <unordered_set>

using namespace std;

#include "transport_catalogue.h"


void TransportCatalogue::AddStop(const string& new_name, const Coordinates& new_coordinates){
    Stop new_stop{new_name, new_coordinates};
    stops_.push_back(std::move(new_stop));
    index_stops_[stops_.back().name] = &stops_.back();
    stops_on_route_[stops_.back().name] = {};
}

const Stop* TransportCatalogue::FindStop(string_view name_stop) const{
    auto iterator = index_stops_.find(name_stop);
    return iterator != index_stops_.end() ? iterator->second : nullptr;
}


void TransportCatalogue::AddDistance(const string& name, const std::vector<std::pair<string, size_t>>& distance){
    for(const auto& pair : distance){
        distances_.insert({{name, pair.first}, pair.second});
    }
}


size_t TransportCatalogue::GetDistance(const string& from, const string& to) const{
    if(distances_.find({from, to}) != distances_.end()){
        return distances_.at({from, to});
    } else{
        return distances_.at({to, from});
    }
}


void TransportCatalogue::AddBus(const string& name, const vector<string_view>& route){
    /*
     * Сначала проверяем, есть ли проверяемая остановка, только потом добавляем.
     * В алгоритме применения команд применена логика, сначала добавляются остановки, потом маршруты
     */

    Stops stops;
    for (const auto& stop_name : route) {
        if (FindStop(stop_name) != nullptr) {
            stops.push_back(index_stops_[stop_name]);
        }
    }

    //Заполняем список: остановка - автобусы, которые через неё ходят.
    for(const auto& stop : stops){
        if(stops_on_route_.find(stop -> name) != stops_on_route_.end()){
            stops_on_route_[stop -> name].insert(name);
        }
    }

    bus_.push_back({name, std::move(stops)});
    index_bus_[bus_.back().name] = &bus_.back();
}

const Bus* TransportCatalogue::FindBus(string_view name_bus) const{
    auto iterator = index_bus_.find(name_bus);
    return iterator != index_bus_.end() ? iterator -> second : nullptr;
}

const set<string>& TransportCatalogue::GetBusByStop(string_view name_stop) const{
    if(stops_on_route_.find(name_stop) != stops_on_route_.end()){
        return stops_on_route_.at(name_stop);
    }

    static set<string> empty{};
    return empty;

}


size_t TransportCatalogue::ComputeFactDistanceRote(const Bus& bus) const{
    size_t distance = 0;

    if (bus.stops.size() < 2) {
        return 0;
    }

    for(size_t i = 0; i < bus.stops.size()-1; ++i){
        distance += GetDistance(bus.stops[i]->name, bus.stops[i+1]->name);
    }

    return distance;
}


double TransportCatalogue::ComputeGeographicDistanceRote(const Bus& bus) const{
    double distance = 0;

    for(size_t i = 0; i < bus.stops.size()-1; ++i){
        distance += ComputeDistance(bus.stops[i] -> coordinates,  bus.stops[i+1]->coordinates);
    }

    return distance;
}

BusInfo TransportCatalogue::GetInfo(const Bus* bus) const{
    BusInfo result{};

    result.route_length = ComputeFactDistanceRote(*bus);
    result.stops_on_rote = bus->stops.size();
    result.unique_stops = unordered_set<Stop*>(bus->stops.begin(), bus->stops.end()).size();
    result.curvature =  (result.route_length * 1.0)  / ComputeGeographicDistanceRote(*bus);

    return result;
}


