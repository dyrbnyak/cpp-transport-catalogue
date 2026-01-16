#include <unordered_set>
#include <iostream>

using namespace std;

#include "transport_catalogue.h"


void TransportCatalogue::AddStop(const string& new_name, const Coordinates& new_coordinates){
    Stop new_stop{new_name, new_coordinates};
    stops_.push_back(std::move(new_stop));
    index_stops_[stops_.back().name] = &stops_.back();
    stops_on_route[stops_.back().name] = {};
}

const Stop* TransportCatalogue::FindStop(string_view name_stop) const{
    auto it = index_stops_.find(name_stop);
    return it != index_stops_.end() ? it->second : nullptr;
}





void TransportCatalogue::AddBus(const string& name, const vector<string_view>& route){
    /*
     * Сначала проверяем, есть ли проверяемая остановка, только потом добавляем.
     * В алгоритме применения команд применена логика, сначала добавляются остановки, потом маршруты
     */

    Stops stops;
    for (const auto& stop_name : route) {
        if (FindStop(stop_name)) {
            stops.push_back(index_stops_[stop_name]);
        }
    }

    //Заполняем список: остановка - автобусы, которые через неё ходят.
    for(const auto& stop : stops){
        if(stops_on_route.find(stop -> name) != stops_on_route.end()){
            stops_on_route[stop -> name].insert(name);
        }
    }

    bus_.push_back({name, std::move(stops)});
    index_bus_[bus_.back().name] = &bus_.back();
}

const Bus* TransportCatalogue::FindBus(string_view name_bus) const{
    auto it = index_bus_.find(name_bus);
    return it != index_bus_.end() ? it->second : nullptr;
}

const set<string>& TransportCatalogue::GetBusByStop(string_view name_stop) const{
    if(stops_on_route.find(name_stop) != stops_on_route.end()){
        return stops_on_route.at(name_stop);
    }

    static set<string> empty{};
    return empty;

}


double TransportCatalogue::ComputeDistanceRote(const Bus& bus) const{
    double distance = 0;

    for(size_t i = 0; i < bus.stops.size()-1; ++i){
        distance += ComputeDistance(bus.stops[i]->coordinates, bus.stops[i+1]->coordinates);
    }

    return distance;
}

BusInfo TransportCatalogue::GetInfo(const Bus* bus) const{
    BusInfo result{};

    result.route_length = ComputeDistanceRote(*bus);
    result.stops_on_rote = bus->stops.size();
    result.unique_stops = unordered_set<Stop*>(bus->stops.begin(), bus->stops.end()).size();

    return result;
}


