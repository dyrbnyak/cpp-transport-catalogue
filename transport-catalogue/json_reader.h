#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_reader {

void LoadBaseRequests(const json::Document& doc, TransportCatalogue& catalogue, RequestHandler& request_handler);

render::RenderSettings LoadRenderSettings(const json::Document& doc);

json::Document ProcessStatRequests(const json::Document& doc, const RequestHandler& handler);

RouteNames ExtractRouteNames(const json::Document& doc);


}  // namespace json_reader
