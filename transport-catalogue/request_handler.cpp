#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view &bus_name) const{
    const auto *b = GetTransportCatalogue().FindBus(bus_name);

    if (!b) {
        return std::nullopt;
    }

    BusStat result;
    result.stops_on_rote    = b->stops_on_rote;
    result.unique_stops     = b->unique_stops;
    result.route_length     = b->route_length;
    result.curvature        = b->curvature;

    return result;
}

const std::unordered_set<BusPtr>* RequestHandler::GetBusesByStop(const std::string_view &stop_name) const{
    const StopPtr stop = GetTransportCatalogue().FindStop(stop_name);

    if(!stop){
        return nullptr;
    }

    return &GetTransportCatalogue().GetBusByStop(stop);
}

RoutePtr RequestHandler::GetAllBusAndStop() const{
    return RoutePtr{GetTransportCatalogue().GetBus(), GetTransportCatalogue().GetStop()};
}
