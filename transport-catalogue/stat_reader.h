#pragma once

#include <iosfwd>
#include <string_view>
#include <utility>
#include <string>

#include "transport_catalogue.h"
#include "geo.h"



namespace detail{
struct TypeRequestAndDescription{
    std::string_view type_request;
    std::string_view description;
};

TypeRequestAndDescription SeparateRequest(std::string_view request);

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output);

void ProcessStatRequests(std::istream& input, const TransportCatalogue& catalogue, std::ostream& output);

void PrintBusInfo(std::ostream& output, const TransportCatalogue& transport_catalogue, const TypeRequestAndDescription& type_request_and_type_request);

void PrintStopInfo(std::ostream& output, const TransportCatalogue& transport_catalogue, const TypeRequestAndDescription& type_request_and_type_request);
}

