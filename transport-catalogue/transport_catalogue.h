#pragma once

#include <deque>
#include <unordered_map>
#include <vector>
#include <string>
#include <set>
#include <optional>

#include "geo.h"

using std::string_view;
using std::string;

struct Stop{
    string name;
    Coordinates coordinates;
};

using Stops = std::vector<Stop*>;

struct Bus{
    string name;
    Stops stops;
};

struct BusInfo{
    int stops_on_rote;
    int unique_stops;
    double route_length;
};




class TransportCatalogue {
public:
    void AddStop(const string& new_name, const Coordinates& new_coordinates);
    const Stop* FindStop(string_view name_stop) const;


    void AddBus(const string& name, const std::vector<string_view>& route);
    const Bus* FindBus(string_view name_bus) const;
    const std::set<string>& GetBusByStop(string_view name_stop) const;
    BusInfo GetInfo(string_view bus) const;

private:
    std::deque<Stop> stops_;
    std::unordered_map<string_view, Stop*> index_stops_;

    std::deque<Bus> bus_;
    std::unordered_map<string_view, Bus*> index_bus_;

    std::unordered_map<string_view, std::set<string>> stops_on_route;

    double ComputeDistanceRote(const Bus& bus) const;
};
