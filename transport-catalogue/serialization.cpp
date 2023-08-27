#include "serialization.h"

using namespace std;
 
namespace serialization {
    
template <typename It>
uint32_t calculate_id(It start, It end, string_view name) {
 
    auto stop_it = find_if(start, end, [&name](const domain::Stop stop) {
    return stop.name == name; // 0
    });
    return distance(start, stop_it); // позиция найденного элемента относительно начала диапазона
}
 
transport_catalogue_protobuf::TransportCatalogue transport_catalogue_serialization(const transport_catalogue::TransportCatalogue& transport_catalogue) {
    
    transport_catalogue_protobuf::TransportCatalogue transport_catalogue_proto; 
    const auto& stops = transport_catalogue.get_stops(); //  создаются объекты:
    const auto& buses = transport_catalogue.get_buses();
    const auto& distances = transport_catalogue.get_distance();
    
    int id = 0;
    for (const auto& stop : stops) {
 
        transport_catalogue_protobuf::Stop stop_proto; 
        stop_proto.set_id_(id);
        stop_proto.set_name_(stop.name);
        stop_proto.set_latitude_(stop.latitude);
        stop_proto.set_longitude_(stop.longitude);        
        *transport_catalogue_proto.add_stops() = move(stop_proto); // добавляется в список остановок       
        ++id;
    }
 
    for (const auto& bus : buses) {
 
        transport_catalogue_protobuf::Bus bus_proto; 
        bus_proto.set_name(bus.name);
 
        for (auto stop : bus.stops) {
            uint32_t stop_id = calculate_id(stops.cbegin(), 
                                            stops.cend(), 
                                            stop->name);
            bus_proto.add_stops(stop_id); // вычисляется идентификационный номер
        }
 
        bus_proto.set_is_roundtrip(bus.is_roundtrip); // круговой маршрут автобуса
        bus_proto.set_route_length(bus.route_length); // длина маршрута
        *transport_catalogue_proto.add_buses() = move(bus_proto); // добавляется в список автобусов
    }
    // цикл по парам остановок и расстояниям из списка
    for (const auto& [pair_stops, pair_distance] : distances) {
 
        transport_catalogue_protobuf::Distance distance_proto; 
        distance_proto.set_start(calculate_id(stops.cbegin(), 
                                              stops.cend(), 
                                              pair_stops.first->name));                                                  distance_proto.set_end(calculate_id(stops.cbegin(), 
                                            stops.cend(), 
                                            pair_stops.second->name));                                                    distance_proto.set_distance(pair_distance); 
        *transport_catalogue_proto.add_distances() = move(distance_proto); // добавляется в список расстояний
    } 
    return transport_catalogue_proto;
}
 // 1
    
transport_catalogue::TransportCatalogue transport_catalogue_deserialization(const transport_catalogue_protobuf::TransportCatalogue& transport_catalogue_proto) {
 
    transport_catalogue::TransportCatalogue transport_catalogue;    
    const auto& stops_proto = transport_catalogue_proto.stops(); // списки остановок, автобусов и расстояний
    const auto& buses_proto = transport_catalogue_proto.buses();
    const auto& distances_proto = transport_catalogue_proto.distances();
    
    for (const auto& stop : stops_proto) {
        
        domain::Stop tc_stop; //создается объект         
        tc_stop.name = stop.name_(); // запись данных:
        tc_stop.latitude = stop.latitude_();
        tc_stop.longitude = stop.longitude_();        
        transport_catalogue.add_stop(move(tc_stop)); // добавляется в список остановок
    }
    
    const auto& tc_stops = transport_catalogue.get_stops(); 
    
    vector<domain::Distance> distances; // для десериализации списка расстояний
    for (const auto& distance : distances_proto) {
        
        domain::Distance tc_distance;        
        tc_distance.start = transport_catalogue.get_stop(tc_stops[distance.start()].name);
        tc_distance.end = transport_catalogue.get_stop(tc_stops[distance.end()].name);        
        tc_distance.distance = distance.distance();        
        distances.push_back(tc_distance); // добавляется в вектор
    }
    
    transport_catalogue.add_distance(distances); // добавляется в `transport_catalogue`      
    
    for (const auto& bus_proto : buses_proto) {  
    
        domain::Bus tc_bus;        
        tc_bus.name = bus_proto.name(); // записывается имя автобуса
 
        for (auto stop_id : bus_proto.stops()) {
            auto name = tc_stops[stop_id].name;            
            tc_bus.stops.push_back(transport_catalogue.get_stop(name));
        } 
        tc_bus.is_roundtrip = bus_proto.is_roundtrip();
        tc_bus.route_length = bus_proto.route_length();        
        transport_catalogue.add_bus(move(tc_bus)); // добавляется в список автобусов
    } 
     return transport_catalogue;
}
 //  2 
transport_catalogue_protobuf::Color color_serialization(const svg::Color& tc_color) {
    
    transport_catalogue_protobuf::Color color_proto; // будут записываться данные сериализации
    
    if (holds_alternative<monostate>(tc_color)) { // проверяет тип
        color_proto.set_none(true); 
        
    } else if (holds_alternative<svg::Rgb>(tc_color)) { // извлекаются значения цветов
        svg::Rgb rgb = get<svg::Rgb>(tc_color);        
        color_proto.mutable_rgb()->set_red_(rgb.red_);
        color_proto.mutable_rgb()->set_green_(rgb.green_);
        color_proto.mutable_rgb()->set_blue_(rgb.blue_);
        
    } else if (holds_alternative<svg::Rgba>(tc_color)) { // извлекаются значения цветов и прозрачности
        svg::Rgba rgba = std::get<svg::Rgba>(tc_color);        
        color_proto.mutable_rgba()->set_red_(rgba.red_);
        color_proto.mutable_rgba()->set_green_(rgba.green_);
        color_proto.mutable_rgba()->set_blue_(rgba.blue_);
        color_proto.mutable_rgba()->set_opacity_(rgba.opacity_);
        
    } else if (holds_alternative<string>(tc_color)) {
        color_proto.set_string_color(get<string>(tc_color)); // записывается в виде строки
    }
    
    return color_proto;
}
 
svg::Color color_deserialization(const transport_catalogue_protobuf::Color& color_proto) {
    
    svg::Color color;
    
    if (color_proto.has_rgb()) { // извлекаются цветa и записываются
        svg::Rgb rgb;        
        rgb.red_ = color_proto.rgb().red_();
        rgb.green_ = color_proto.rgb().green_();
        rgb.blue_ = color_proto.rgb().blue_();        
        color = rgb;
        
    } else if (color_proto.has_rgba()) { // извлекаются цветa и прозрачности и записываются
        svg::Rgba rgba;        
        rgba.red_ = color_proto.rgba().red_();
        rgba.green_ = color_proto.rgba().green_();
        rgba.blue_ = color_proto.rgba().blue_();
        rgba.opacity_ = color_proto.rgba().opacity_();        
        color = rgba;
        
    } else {color = color_proto.string_color();}    
    return color;
}
 // 3   
transport_catalogue_protobuf::RenderSettings render_settings_serialization(const map_renderer::RenderSettings& render_settings) { // преобразует объект 
    
    transport_catalogue_protobuf::RenderSettings render_settings_proto; // Создается объект
    // поля из объекта копируются в соответствующие поля     
    render_settings_proto.set_height_(render_settings.height_);
    render_settings_proto.set_padding_(render_settings.padding_);
    render_settings_proto.set_line_width_(render_settings.line_width_);
    render_settings_proto.set_stop_radius_(render_settings.stop_radius_);
    render_settings_proto.set_bus_label_font_size_(render_settings.bus_label_font_size_); 
    //  Создается объект и ему присваиваются значения из поля
    transport_catalogue_protobuf::Point bus_label_offset_proto;
    // Перемещением объекта происходит копирование значений координат `x` и `y`
    bus_label_offset_proto.set_x(render_settings.bus_label_offset_.first);
    bus_label_offset_proto.set_y(render_settings.bus_label_offset_.second);    
    *render_settings_proto.mutable_bus_label_offset_() = move(bus_label_offset_proto);
    // аналогичным образом записываются соответствующие значения в объекте
    render_settings_proto.set_stop_label_font_size_(render_settings.stop_label_font_size_); 
    transport_catalogue_protobuf::Point stop_label_offset_proto;
    stop_label_offset_proto.set_x(render_settings.stop_label_offset_.first);
    stop_label_offset_proto.set_y(render_settings.stop_label_offset_.second);    
    *render_settings_proto.mutable_stop_label_offset_() = move(stop_label_offset_proto);
    *render_settings_proto.mutable_underlayer_color_() = move(color_serialization(render_settings.underlayer_color_));
    render_settings_proto.set_underlayer_width_(render_settings.underlayer_width_);
    
    const auto& colors = render_settings.color_palette_;
    for (const auto& color : colors) {
        *render_settings_proto.add_color_palette_() = move(color_serialization(color));
    }
 
    return render_settings_proto;
}
  // 4  
map_renderer::RenderSettings render_settings_deserialization(const transport_catalogue_protobuf::RenderSettings& render_settings_proto) {
    
    map_renderer::RenderSettings render_settings; // Создается объект
    // копируются в соответствующие поля
    render_settings.width_ = render_settings_proto.width_();
    render_settings.height_ = render_settings_proto.height_();
    render_settings.padding_ = render_settings_proto.padding_();
    render_settings.line_width_ = render_settings_proto.line_width_();
    render_settings.stop_radius_ = render_settings_proto.stop_radius_();
    render_settings.bus_label_font_size_ = render_settings_proto.bus_label_font_size_();    
    render_settings.bus_label_offset_.first = render_settings_proto.bus_label_offset_().x();
    render_settings.bus_label_offset_.second = render_settings_proto.bus_label_offset_().y();    
    render_settings.stop_label_font_size_ = render_settings_proto.stop_label_font_size_();    
    render_settings.stop_label_offset_.first = render_settings_proto.stop_label_offset_().x();
    render_settings.stop_label_offset_.second = render_settings_proto.stop_label_offset_().y();
    // вызов функции для преобразования строки
    render_settings.underlayer_color_ = color_deserialization(render_settings_proto.underlayer_color_());
    render_settings.underlayer_width_ = render_settings_proto.underlayer_width_();
    
    for (const auto& color_proto : render_settings_proto.color_palette_()) {  // итерация по списку
        render_settings.color_palette_.push_back(color_deserialization(color_proto));
    }    
    return render_settings;
} 
 // 5
transport_catalogue_protobuf::RoutingSettings routing_settings_serialization(const domain::RoutingSettings& routing_settings) {
 
    transport_catalogue_protobuf::RoutingSettings routing_settings_proto; // Создается объект
    // Значения полей копируются в соответствующие поля объекта proto
    routing_settings_proto.set_bus_wait_time(routing_settings.bus_wait_time);
    routing_settings_proto.set_bus_velocity(routing_settings.bus_velocity);
 
    return routing_settings_proto;
}
    
domain::RoutingSettings routing_settings_deserialization(const transport_catalogue_protobuf::RoutingSettings& routing_settings_proto) {
    
    domain::RoutingSettings routing_settings;
    // Значения полей из объекта proto копируются в соответствующие поля 
    routing_settings.bus_wait_time = routing_settings_proto.bus_wait_time();
    routing_settings.bus_velocity = routing_settings_proto.bus_velocity();
    
    return routing_settings;
}
 // сериализацию объектов в прото и записывает его в поток   
void catalogue_serialization(const transport_catalogue::TransportCatalogue& transport_catalogue, 
                             const map_renderer::RenderSettings& render_settings, 
                             const domain::RoutingSettings& routing_settings, 
                             ostream& out) {
    
    transport_catalogue_protobuf::Catalogue catalogue_proto;
 // выполняется сериализация объектов 
    transport_catalogue_protobuf::TransportCatalogue transport_catalogue_proto = transport_catalogue_serialization(transport_catalogue);
    transport_catalogue_protobuf::RenderSettings render_settings_proto = render_settings_serialization(render_settings);
    transport_catalogue_protobuf::RoutingSettings routing_settings_proto = routing_settings_serialization(routing_settings);
 // прото перемещаются в соответствующие поля объекта 
    *catalogue_proto.mutable_transport_catalogue() = move(transport_catalogue_proto);
    *catalogue_proto.mutable_render_settings() = move(render_settings_proto);
    *catalogue_proto.mutable_routing_settings() = move(routing_settings_proto);
    // запись сериализованных данных в поток
    catalogue_proto.SerializePartialToOstream(&out); 
}
 // выполняет десериализацию объекта из потока   
Catalogue catalogue_deserialization(istream& in) {
    
    transport_catalogue_protobuf::Catalogue catalogue_proto;
    auto success_parsing_catalogue_from_istream = catalogue_proto.ParseFromIstream(&in);
    
    if (!success_parsing_catalogue_from_istream) {
        throw runtime_error("cannot parse serialized file from istream");
    }
    
    return {transport_catalogue_deserialization(catalogue_proto.transport_catalogue()),
            render_settings_deserialization(catalogue_proto.render_settings()),
            routing_settings_deserialization(catalogue_proto.routing_settings())};
}
    
}//end namespace serialization
