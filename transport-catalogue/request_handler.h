#pragma once

#include "transport_catalogue.h"
#include "transport_router.h"
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
    RequestHandler() = default;

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Возвращает указатели на все маршруты и остановки
    RoutePtr GetAllBusAndStop() const;


    void SetTransportCatalogue(const TransportCatalogue& catalogue) {
        transport_catalogue_ = &catalogue;
    }

    void SetRouter(const TransportRouter& router) {
        router_ = &router;
    }

    void SetRenderer(const render::MapRenderer& renderer) {
        renderer_ = &renderer;
    }

    // Get методы с проверками (бросают исключение)
    const TransportCatalogue& GetTransportCatalogue() const {
        if (!transport_catalogue_) {
            throw std::runtime_error("TransportCatalogue not initialized");
        }
        return *transport_catalogue_;
    }

    const TransportRouter& GetRouter() const {
        if (!router_) {
            throw std::runtime_error("Router not initialized");
        }
        return *router_;
    }

    const render::MapRenderer& GetRenderer() const {
        if (!renderer_) {
            throw std::runtime_error("Renderer not initialized");
        }
        return *renderer_;
    }

private:
    const TransportCatalogue* transport_catalogue_ = nullptr;
    const TransportRouter* router_ = nullptr;
    const render::MapRenderer* renderer_ = nullptr;
};

