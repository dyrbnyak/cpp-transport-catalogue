#include <unordered_set>
#include <iostream>

using namespace std;

#include "transport_catalogue.h"


void TransportCatalogue::AddStop(const string& new_name, const Coordinates& new_coordinates){
    Stop new_stop{new_name, new_coordinates};
    stops_.push_back(std::move(new_stop));
    index_stops_[stops_.back().name] = &stops_.back();
}

bool TransportCatalogue::FindStop(const string_view &name_stop) const{
    return index_stops_.count(name_stop) > 0 ? true : false;
}

const unordered_map<string_view, Stop *> &TransportCatalogue::GetStops() const{
    return index_stops_;
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

        } else {
            cerr << "Stop not found: " << stop_name << endl;
        }
    }

    bus_.push_back({name, std::move(stops)});
    index_bus_[bus_.back().name] = &bus_.back();
}

bool TransportCatalogue::FindBus(const string_view &name_bus) const{
    return index_bus_.count(name_bus) > 0 ? true : false;
}

const set<string> TransportCatalogue::GetStopsByBus(const string_view &name_bus) const{
    set<string> result;

    for(const auto& [name, bus] : index_bus_){
        for(const auto& stop_ptr : bus->stops){
            if(stop_ptr->name == name_bus){
                result.insert(bus->name);
                break;
            }
        }
    }

    return result;
}


const unordered_map<string_view, Bus *> &TransportCatalogue::GetBus() const{
    return index_bus_;
}



double TransportCatalogue::ComputeDistanceRote(const Bus& bus) const{
    double distance = 0;

    for(size_t i = 0; i < bus.stops.size()-1; ++i){
        distance += ComputeDistance(bus.stops[i]->coordinates, bus.stops[i+1]->coordinates);
    }

    return distance;
}

BusInfo TransportCatalogue::GetInfo(const string_view &bus) const{
    BusInfo result{};

    result.route_length = ComputeDistanceRote(*index_bus_.at(bus));
    result.stops_on_rote = index_bus_.at(bus)->stops.size();
    result.unique_stops = unordered_set<Stop*>(index_bus_.at(bus)->stops.begin(), index_bus_.at(bus)->stops.end()).size();

    return result;
}


