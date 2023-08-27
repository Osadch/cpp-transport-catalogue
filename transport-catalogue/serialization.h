#pragma once
 
#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"
 
#include "svg.pb.h"
 
#include "map_renderer.h"
#include "map_renderer.pb.h"
 
#include "transport_router.h"
#include "transport_router.pb.h"
 
#include <iostream>
 
namespace serialization {
    
struct SerializationSettings {std::string file_name;};
// структура для хранения данных каталога    
struct Catalogue {
    transport_catalogue::TransportCatalogue transport_catalogue_;
    map_renderer::RenderSettings render_settings_;
    domain::RoutingSettings routing_settings_;
};
    
template <typename It>
uint32_t calculate_id(It start, It end, std::string_view name); // для вычисления идентификатора    
transport_catalogue_protobuf::TransportCatalogue transport_catalogue_serialization(const transport_catalogue::TransportCatalogue& transport_catalogue); // для сериализации каталога
transport_catalogue::TransportCatalogue transport_catalogue_deserialization(const transport_catalogue_protobuf::TransportCatalogue& transport_catalogue_proto); // для десериализации каталога 
transport_catalogue_protobuf::Color color_serialization(const svg::Color& tc_color); // для сериализации цвета
svg::Color color_deserialization(const transport_catalogue_protobuf::Color& color_proto); // для десериализации цвета 
transport_catalogue_protobuf::RenderSettings render_settings_serialization(const map_renderer::RenderSettings& render_settings); // для сериализации настроек отрисовки
map_renderer::RenderSettings render_settings_deserialization(const transport_catalogue_protobuf::RenderSettings& render_settings_proto); // для десериализации настроек отрисовки   
transport_catalogue_protobuf::RoutingSettings routing_settings_serialization(const domain::RoutingSettings& routing_settings); // для сериализации настроек маршрутизации
domain::RoutingSettings routing_settings_deserialization(const transport_catalogue_protobuf::RoutingSettings& routing_settings_proto); // для десериализации настроек маршрутизации
// для сериализации этих объектов в поток 
void catalogue_serialization(const transport_catalogue::TransportCatalogue& transport_catalogue, 
                             const map_renderer::RenderSettings& render_settings,
                             const domain::RoutingSettings& routing_settings,
                             std::ostream& out); 
    
Catalogue catalogue_deserialization(std::istream& in); // для десериализации объекта "Catalogue" из потока
    
}//end namespace serialization
