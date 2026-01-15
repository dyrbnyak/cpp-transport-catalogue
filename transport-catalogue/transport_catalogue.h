#pragma once

#include <deque>
#include <unordered_map>
#include <vector>
#include <string>
#include <set>

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
    int stops_on_rote;
    int unique_stops;
    double route_length;
};




class TransportCatalogue {
public:
    void AddStop(const std::string& new_name, const Coordinates& new_coordinates);
    bool FindStop(const std::string_view& name_stop) const;
    const std::unordered_map<std::string_view, Stop*>& GetStops() const;


    void AddBus(const std::string& name, const std::vector<std::string_view>& route);
    bool FindBus(const std::string_view& name_bus) const;
    const std::set<std::string> GetStopsByBus(const std::string_view& name_bus) const;
    const std::unordered_map<std::string_view, Bus*>& GetBus() const;

    double ComputeDistanceRote(const Bus& bus) const;

    BusInfo GetInfo(const std::string_view& bus) const;


private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> index_stops_;

    std::deque<Bus> bus_;
    std::unordered_map<std::string_view, Bus*> index_bus_;
};
