#include "map_renderer.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <unordered_set>
using namespace std::literals;

namespace render {

svg::Color MapRenderer::GetColor(size_t idx) const {
    if (settings_.color_palette.empty()) return "black";
    return settings_.color_palette[idx % settings_.color_palette.size()];
}

void MapRenderer::RenderMap(std::ostream& out,
                            const Buses& buses,
                            [[maybe_unused]] const Stops& stops) const {

    svg::Document doc;

    //Собираем координаты по всем остановок
    std::vector<Coordinates> сoordinates = GetCoordinatesFromBuses(buses);

    //Сбор уникальныйх остановок из всех маршрутов
    UniqueStops unique_stops = GetUiqueStops(buses);


    //Проецируем с сферы на плоскость координаты
    SphereProjector proj(сoordinates.begin(), сoordinates.end(),
                         settings_.width, settings_.height, settings_.padding);

    //Отрисовка
    //Первый слой: Отрисовка путей
    RenderFirstLayer(doc, buses, proj);


    //Второй слой: названия маршрутов
    RenderSecondLayer(doc, buses, proj);


    // Слой 3: круги, обозначающие остановки
    RenderThirdLayer(doc, unique_stops, proj);


    //Слой 4: названия остановок
    RenderFourthLayer(doc, unique_stops, proj);


    doc.Render(out);
}



void MapRenderer::RenderFirstLayer(svg::Document &doc, const Buses &buses, const SphereProjector &proj) const{
    for (size_t i = 0; i < buses.size(); ++i) {
        RenderRoute(doc, buses[i], proj, i);
    }
}

void MapRenderer::RenderSecondLayer(svg::Document &doc, const Buses &buses, const SphereProjector &proj) const{
    for (size_t i = 0; i < buses.size(); ++i) {
        const auto* bus = buses[i];

        //Если у маршрута нету остановок - не рисуем
        if (bus->stops.empty()){
            continue;
        }

        //В кольцевом маршруте — когда "is_roundtrip": true — конечной считается первая остановка маршрута.
        //А в некольцевом — первая и последняя. Короче, первая остановка полюбому есть.
        if(bus->is_roundtrip){
            const auto* first_stop = bus->stops.front();
            RenderBusLabel(doc, bus, first_stop, proj, i);
        }

        // Для каждого маршрута сначала выводится название для его первой конечной остановки,
        // а затем, если маршрут некольцевой и конечные не совпадают, — для второй конечной.
        if(!bus->is_roundtrip){
            //Здесь обращаюсь конкретно к остановкам, а не к полному маршруту
            //Но чуть шо, убираю дублирование просто остановок и маршрута
            //и здесь мастерю так, обращаюсь к первому элементу через иттератор
            const auto* first_stop = bus->stops.front();
            const auto* last_stop = bus->stops.back();

            RenderBusLabel(doc, bus, first_stop, proj, i);

            if(last_stop != first_stop){
                RenderBusLabel(doc, bus, last_stop, proj, i);
            }
        }
    }
}

void MapRenderer::RenderThirdLayer(svg::Document& doc, const UniqueStops& unique_stops, const SphereProjector& proj) const{
    //Отрисовка остановок, кружков
    for (const auto* stop : unique_stops) {
        RenderStop(doc, stop, proj);
    }
}

void MapRenderer::RenderFourthLayer(svg::Document &doc, const UniqueStops &unique_stops, const SphereProjector &proj) const{
    //Отрисовка названия
    for (const auto* stop : unique_stops) {
        RenderStopLabel(doc, stop, proj);
    }
}



void MapRenderer::RenderRoute(svg::Document& doc, const Bus* bus, const SphereProjector& proj, size_t idx) const {
    if (bus->stops.empty()){
        return;
    }

    svg::Polyline pl;

    pl.SetStrokeColor(GetColor(idx))
        .SetFillColor(svg::NoneColor)
        .SetStrokeWidth(settings_.line_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);


    //У реализация добавления остановок такая,
    //что маршрут хранит полный список остановок.
    //Таким образом мне не надо реализовывать доп логику обработки
    //кольцевого маршрута или не колцевого
    //
    //Ознакомиться с реализацией в TransportCatalogue::AddBus
    for (const auto& s : bus->full_route){
        pl.AddPoint(proj(s->coordinates));
    }


    doc.Add(std::move(pl));
}

void MapRenderer::RenderBusLabel(svg::Document& doc,
                                 const Bus* bus,
                                 const Stop* stop,
                                 const SphereProjector& proj,
                                 size_t idx) const {

    svg::Point pos = proj(stop->coordinates);

    // Подложка
    svg::Text bg;
    bg.SetPosition(pos)
        .SetOffset({settings_.bus_label_offset[0], settings_.bus_label_offset[1]})
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(bus->name)
        .SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    doc.Add(std::move(bg));

    // Текст
    svg::Text txt;
    txt.SetPosition(pos)
        .SetOffset({settings_.bus_label_offset[0], settings_.bus_label_offset[1]})
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(bus->name)
        .SetFillColor(GetColor(idx));
    doc.Add(std::move(txt));
}

void MapRenderer::RenderStop(svg::Document& doc,
                             const Stop* stop,
                             const SphereProjector& proj) const {
    svg::Circle c;

    c.SetCenter(proj(stop->coordinates))
        .SetRadius(settings_.stop_radius)
        .SetFillColor("white");

    doc.Add(std::move(c));
}

void MapRenderer::RenderStopLabel(svg::Document& doc,
                                  const Stop* stop,
                                  const SphereProjector& proj) const {


    svg::Point pos = proj(stop->coordinates);

    // Подложка
    svg::Text bg;
    bg.SetPosition(pos)
        .SetOffset({settings_.stop_label_offset[0], settings_.stop_label_offset[1]})
        .SetFontSize(settings_.stop_label_font_size)
        .SetFontFamily("Verdana")
        .SetData(stop->name)
        .SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    doc.Add(std::move(bg));

    // Текст
    svg::Text txt;
    txt.SetPosition(pos)
        .SetOffset({settings_.stop_label_offset[0], settings_.stop_label_offset[1]})
        .SetFontSize(settings_.stop_label_font_size)
        .SetFontFamily("Verdana")
        .SetData(stop->name)
        .SetFillColor("black");

    doc.Add(std::move(txt));
}

std::string MapRenderer::RenderMapToString(const Buses &buses, const std::vector<StopPtr> &stops) const{
    std::ostringstream out_string;
    RenderMap(out_string, buses, stops);
    return out_string.str();
}

}  // namespace render
