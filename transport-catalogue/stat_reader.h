#pragma once

#include <iosfwd>
#include <string_view>
#include <utility>
#include <string>

#include "transport_catalogue.h"
#include "geo.h"



namespace detail{
std::pair<std::string_view, std::string_view> SeparateRequest(std::string_view request);


void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output);
}

