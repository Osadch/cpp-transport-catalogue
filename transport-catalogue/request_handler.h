#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <sstream>


 class RequestHandler {
 public:
     // MapRenderer понадобится в следующей части итогового проекта
     RequestHandler(const transport::TransportCatalogue& catalogue, const renderer::MapRenderer& renderer)
         : catalogue_(catalogue)
         , renderer_(renderer)
     {
     }

     // Возвращает информацию о маршруте (запрос Bus)
     std::optional<transport::BusStat> GetBusStat(const std::string_view bus_number) const;
     // Возвращает маршруты, проходящие через
     const std::set<std::string> GetBusesByStop(std::string_view stop_name) const;

     bool IsBusNumber(const std::string_view bus_number) const;

     bool IsStopName(const std::string_view stop_name) const;

     // Этот метод будет нужен в следующей части итогового проекта
     svg::Document RenderMap() const;

 private:
     // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
     const transport::TransportCatalogue& catalogue_;
     const renderer::MapRenderer& renderer_;
 };
 
