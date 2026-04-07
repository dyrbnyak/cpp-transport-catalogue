#include "json_reader.h"
#include "json_builder.h"
#include "json.h"
#include "map_renderer.h"


#include <cstdint>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include <algorithm>
#include <cmath>

using namespace std::literals;

namespace json_reader {

json::Node ProcessBusRequest(const json::Dict& request, const RequestHandler& handler);
json::Node ProcessStopRequest(const json::Dict& request, const RequestHandler& handler);
json::Node ProcessMapRequest(const json::Dict& request, const RequestHandler& handler);

void LoadStop(const json::Dict& stop_dict, TransportCatalogue& catalogue) {
    std::string name    = stop_dict.at("name").AsString();
    double latitude     = stop_dict.at("latitude").AsDouble();
    double longitude    = stop_dict.at("longitude").AsDouble();

    catalogue.AddStop(name, {latitude, longitude});
}

void SetRoadDistances(const json::Dict& stop_dict, TransportCatalogue& catalogue) {
    const std::string& stop_name = stop_dict.at("name").AsString();

    if (auto stop_ptr = catalogue.FindStop(stop_name)){
        const auto& road_distances = stop_dict.at("road_distances").AsDict();
        for (const auto& [to, distance] : road_distances) {

            if (auto to_ptr = catalogue.FindStop(to); to_ptr) {
                catalogue.AddDistance(stop_ptr -> name, to_ptr -> name, distance.AsDouble());
            }
        }
    }
}

void LoadBus(const json::Dict& bus_dict, TransportCatalogue& catalogue) {
    std::string name = bus_dict.at("name").AsString();
    bool is_roundtrip = bus_dict.at("is_roundtrip").AsBool();

    std::vector<StopPtr> stops;

    for (const auto& stop_node : bus_dict.at("stops").AsArray()) {
        if (auto stop_ptr = catalogue.FindStop(stop_node.AsString()); stop_ptr){
            stops.push_back(stop_ptr);
        }
    }



    catalogue.AddBus(name, stops, is_roundtrip);
}

void LoadBaseRequests(const json::Document& doc, TransportCatalogue& catalogue, RequestHandler& request_handler) {
    /*
     * Сначала считываем остановки, дистанции, затем маршруты.
     * В методе добавления маршрута заложена проверка, существует ли остановка,
     * переданная в параметрах, для этого должны быть заранее считаны все остановки.
    */
    const auto& root = doc.GetRoot().AsDict();
    const auto& base_requests = root.at("base_requests").AsArray();

    //Добавление настроек рендера в requset handler
    render::RenderSettings render_settings = json_reader::LoadRenderSettings(doc);
    request_handler.SetRenderSetting(render_settings);

    //Добавление остановок
    for (const auto& node : base_requests) {
        const auto& dict = node.AsDict();
        if (dict.at("type").AsString() == "Stop"){
            LoadStop(dict, catalogue);
        }
    }

    //Добавление расстояний
    for (const auto& node : base_requests) {
        const auto& dict = node.AsDict();
        if (dict.at("type").AsString() == "Stop"){
            SetRoadDistances(dict, catalogue);
        }
    }

    //Добавление маршрутов
    for (const auto& node : base_requests) {
        const auto& dict = node.AsDict();
        if (dict.at("type").AsString() == "Bus"){
            LoadBus(dict, catalogue);
        }
    }
}

json::Node ProcessBusRequest(const json::Dict& request, const RequestHandler& handler) {
    json::Dict response;
    int request_id = request.at("id").AsInt();
    response["request_id"] = request_id;

    const std::string& bus_name = request.at("name").AsString();
    auto stat_opt = handler.GetBusStat(bus_name);

    if (!stat_opt.has_value()) {
        response["error_message"] = std::string("not found");
        return json::Node(std::move(response));
    }

    response["stop_count"] = stat_opt->stops_on_rote;
    response["unique_stop_count"] = stat_opt->unique_stops;
    response["route_length"] = stat_opt->route_length;
    response["curvature"] = stat_opt->curvature;

    return json::Node(std::move(response));
}

json::Node ProcessStopRequest(const json::Dict& request, const RequestHandler& handler) {
    json::Dict response;
    response["request_id"] = request.at("id").AsInt();

    std::string_view stop_name = request.at("name").AsString();
    auto stop_ptr = handler.GetBusesByStop(stop_name);

    if (stop_ptr == nullptr) {
        response["error_message"] = std::string("not found");
        return json::Node(std::move(response));
    }

    std::set<std::string> bus_names;
    for (const auto* bus : *stop_ptr) {
        bus_names.insert(bus -> name);
    }

    json::Array buses_array;
    buses_array.reserve(bus_names.size());
    for (const auto& name : bus_names) {
        buses_array.push_back(name);
    }

    response["buses"] = json::Node(std::move(buses_array));
    return json::Node(std::move(response));
}

json::Node ProcessMapRequest(const json::Dict& request, const RequestHandler& handler){
    json::Dict response;

    response["request_id"] = request.at("id").AsInt();

    render::MapRenderer renderer(handler.GetRenderSettings().value());

    RoutePtr route_ptr = handler.GetAllBusAndStop();

    std::sort(route_ptr.bus_ptr.begin(), route_ptr.bus_ptr.end(),
              [](BusPtr a, BusPtr b) { return a->name < b->name; });

    // Получаем карту как строку
    std::string map_svg = renderer.RenderMapToString(route_ptr.bus_ptr,
                                                     route_ptr.stop_ptr);

    response["map"] = map_svg;

    return json::Node(std::move(response));
}

json::Document ProcessStatRequests(const json::Document& doc, const RequestHandler& handler) {
    const auto& root = doc.GetRoot().AsDict();
    const auto& stat_requests = root.at("stat_requests").AsArray();

    json::Array responses;
    responses.reserve(stat_requests.size());

    for (const auto& request_node : stat_requests) {
        const auto& request = request_node.AsDict();
        const std::string& type = request.at("type").AsString();

        if (type == "Bus") {
            responses.emplace_back(ProcessBusRequest(request, handler));

        } else if (type == "Stop") {
            responses.emplace_back(ProcessStopRequest(request, handler));

        } else if (type == "Map") {
            responses.emplace_back(ProcessMapRequest(request, handler));
        }
    }

    return json::Document(json::Node(std::move(responses)));
}



RouteNames ExtractRouteNames(const json::Document& doc) {
    RouteNames route_names;
    const auto& root = doc.GetRoot().AsDict();


    auto it = root.find("base_requests");
    if (it == root.end() || !it->second.IsArray()){
        return route_names;
    }

    for (const auto& req : it->second.AsArray()) {
        const auto& m = req.AsDict();
        const std::string& type = m.at("type").AsString();
        const std::string& name = m.at("name").AsString();


        if (type == "Bus"){
            route_names.bus_names.push_back(name);

        } else if (type == "Stop"){
            route_names.stop_names.push_back(name);
        }
    }
    return route_names;
}

render::RenderSettings LoadRenderSettings(const json::Document& doc) {
    render::RenderSettings settings;
    const auto& root = doc.GetRoot().AsDict();

    auto it = root.find("render_settings");
    if (it == root.end() || !it->second.IsDict()){
        return settings;
    }

    const auto& rs = it->second.AsDict();

    //Инициализация параметров
    if (auto w = rs.find("width"); w != rs.end() && w->second.IsDouble()){
        settings.width = w->second.AsDouble();
    }

    if (auto h = rs.find("height"); h != rs.end() && h->second.IsDouble()){
        settings.height = h->second.AsDouble();
    }

    if (auto p = rs.find("padding"); p != rs.end() && p->second.IsDouble()){
        settings.padding = p->second.AsDouble();
    }

    if (auto lw = rs.find("line_width"); lw != rs.end() && lw->second.IsDouble()){
        settings.line_width = lw->second.AsDouble();
    }

    if (auto sr = rs.find("stop_radius"); sr != rs.end() && sr->second.IsDouble()){
        settings.stop_radius = sr->second.AsDouble();
    }

    if (auto blfs = rs.find("bus_label_font_size"); blfs != rs.end() && blfs->second.IsInt()){
        settings.bus_label_font_size = blfs->second.AsInt();
    }

    if (auto blo = rs.find("bus_label_offset"); blo != rs.end() && blo->second.IsArray()) {
        //Очищаем,
        //тк при создании структуры данный вектор
        //уже проинициализирован элементами
        settings.bus_label_offset.clear();

        for (const auto& v : blo->second.AsArray()){
            settings.bus_label_offset.push_back(v.AsDouble());
        }
    }

    if (auto slfs = rs.find("stop_label_font_size"); slfs != rs.end() && slfs->second.IsInt()){
        settings.stop_label_font_size = slfs->second.AsInt();
    }

    if (auto slo = rs.find("stop_label_offset"); slo != rs.end() && slo->second.IsArray()) {
        //Очищаем,
        //тк при создании структуры данный вектор
        //уже проинициализирован элементами
        settings.stop_label_offset.clear();

        for (const auto& v : slo->second.AsArray()){
            settings.stop_label_offset.push_back(v.AsDouble());
        }
    }

    if (auto uc_it = rs.find("underlayer_color"); uc_it != rs.end()) {
        if (uc_it->second.IsString()) {
            //Обрабатывае случай, когда цвет передан названием цвета
            settings.underlayer_color = uc_it->second.AsString();


        } else if (uc_it->second.IsArray()) {
            //Обрабатываем случай, когда это RGB или RGBA
            const auto& arr = uc_it->second.AsArray();


            if (arr.size() == 3) {
                uint8_t r = static_cast<uint8_t>(arr[0].AsDouble());
                uint8_t g = static_cast<uint8_t>(arr[1].AsDouble());
                uint8_t b = static_cast<uint8_t>(arr[2].AsDouble());

                settings.underlayer_color = svg::Rgb{r, g, b};
            }

            if (arr.size() == 4) {
                uint8_t r   = static_cast<uint8_t>(arr[0].AsDouble());
                uint8_t g   = static_cast<uint8_t>(arr[1].AsDouble());
                uint8_t b   = static_cast<uint8_t>(arr[2].AsDouble());
                double a    = static_cast<double>(arr[3].AsDouble());

                settings.underlayer_color = svg::Rgba{r, g, b, a};
            }
        }
    }

    if (auto uw = rs.find("underlayer_width"); uw != rs.end() && uw->second.IsDouble()){
        settings.underlayer_width = uw->second.AsDouble();
    }

    if (auto cp_it = rs.find("color_palette"); cp_it != rs.end() && cp_it->second.IsArray()) {
        //Очищаем,
        //тк при создании структуры данный вектор
        //уже проинициализирован элементами
        settings.color_palette.clear();

        for (const auto& c : cp_it->second.AsArray()) {
            if (c.IsString()) {
                //Обрабатывае случай, когда цвет политры передан названием цвета
                settings.color_palette.push_back(c.AsString());

            } else if (c.IsArray()) {
                //Обрабатываем случай, когда это RGB или RGBA
                const auto& arr = c.AsArray();

                if (arr.size() == 3) {
                    uint8_t r = static_cast<uint8_t>(arr[0].AsDouble());
                    uint8_t g = static_cast<uint8_t>(arr[1].AsDouble());
                    uint8_t b = static_cast<uint8_t>(arr[2].AsDouble());

                    settings.color_palette.push_back(svg::Rgb{r, g, b});
                }

                if (arr.size() == 4) {
                    uint8_t r   = static_cast<uint8_t>(arr[0].AsDouble());
                    uint8_t g   = static_cast<uint8_t>(arr[1].AsDouble());
                    uint8_t b   = static_cast<uint8_t>(arr[2].AsDouble());
                    double  a   = static_cast<double>(arr[3].AsDouble());

                    settings.color_palette.push_back(svg::Rgba{r, g, b, a});
                }
            }
        }
    }

    return settings;
}

}  // namespace json_reader
