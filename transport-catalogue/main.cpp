#include "json_builder.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include <iostream>
#include <algorithm>

using namespace std::literals;

int main() {
    json::Document doc = json::Load(std::cin);

    RequestHandler request_handler;
    TransportCatalogue catalogue;
    render::MapRenderer renderer(json_reader::LoadRenderSettings(doc));

    json_reader::LoadBaseRequests(doc, catalogue);

    //Router должен быть создан после заполнения в catalogue
    TransportRouter router(catalogue, json_reader::LoadRoutingSettings(doc));

    request_handler.SetRouter(router);
    request_handler.SetTransportCatalogue(catalogue);
    request_handler.SetRenderer(renderer);


    RoutePtr route_ptr = request_handler.GetAllBusAndStop();

    std::sort(route_ptr.bus_ptr.begin(), route_ptr.bus_ptr.end(),
              [](BusPtr a, BusPtr b) {
                  return (a->name) < (b->name);
              });


    json::Print(json_reader::ProcessStatRequests(doc, request_handler), std::cout);
 }

