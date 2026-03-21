#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include <iostream>
#include <algorithm>

using namespace std::literals;

int main() {
    json::Document doc = json::Load(std::cin);

    TransportCatalogue catalogue;
    RequestHandler request_handler(catalogue);
    json_reader::LoadBaseRequests(doc, catalogue, request_handler);

    render::RenderSettings render_settings = json_reader::LoadRenderSettings(doc);
    render::MapRenderer renderer(render_settings);


    RoutePtr route_ptr = request_handler.GetAllBusAndStop();

    std::sort(route_ptr.bus_ptr.begin(), route_ptr.bus_ptr.end(),
              [](BusPtr a, BusPtr b) {
                  return (a->name) < (b->name);
              });

    json::Print(json_reader::ProcessStatRequests(doc, request_handler), std::cout);
 }
