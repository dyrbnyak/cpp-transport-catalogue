#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */


/*
 *
 * В моей архитектуре json_reader не может быть классом, так как у него нет объектов композиции
 * json_reder - "библиотека" для взаимодествяи с json в рамках проекта "Транспортный Справочник"
 *
 * В свою очередь request_handler должен быть классом, так как имеется агрегацию map_rander и data_bases,
 * Является фундаментов для реализации патерна проектирования "Фасад"
 */

namespace json_reader {
// Объявление функция обработки запросов
json::Node ProcessBusRequest    (const json::Dict& request, const RequestHandler& handler);
json::Node ProcessStopRequest   (const json::Dict& request, const RequestHandler& handler);
json::Node ProcessMapRequest    (const json::Dict& request, const RequestHandler& handler);
json::Node ProcessRouteRequest  (const json::Dict& request, const RequestHandler& handler);

//Загрузка входных данных в базу данных
void LoadBaseRequests(const json::Document& doc, TransportCatalogue& catalogue);

//Загрузка настроек рендера
render::RenderSettings LoadRenderSettings(const json::Document& doc);

//Загрузка настроек маршрута
RoutingSettings LoadRoutingSettings(const json::Document& doc);

//Сбор финального вывода данных
json::Document ProcessStatRequests(const json::Document& doc, const RequestHandler& handler);

//Сбор имен маршрутов
RouteNames ExtractRouteNames(const json::Document& doc);

}  // namespace json_reader
