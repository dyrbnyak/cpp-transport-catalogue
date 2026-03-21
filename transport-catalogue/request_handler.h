#pragma once

#include "transport_catalogue.h"
#include "svg.h"
#include "map_renderer.h"

#include <unordered_set>

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& db)
        : db_(db), render_settings_(std::nullopt) {}

    RequestHandler(const TransportCatalogue& db, const render::RenderSettings& settings)
        : db_(db), render_settings_(settings) {}

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Возвращает указатели на все маршруты и остановки
    RoutePtr GetAllBusAndStop() const;

    //Возвращает настройки рендера
    const std::optional<render::RenderSettings>& GetRenderSettings() const;

    //Установка настроек рендера
    void SetRenderSetting(const render::RenderSettings& render_settings);

private:
    const TransportCatalogue& db_;
    std::optional<render::RenderSettings> render_settings_;
};

