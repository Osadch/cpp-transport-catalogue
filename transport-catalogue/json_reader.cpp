#include "json_reader.h"

const json::Node& JsonReader::GetBaseRequests() const {
    auto it = input_.GetRoot().AsMap().find("base_requests");
    if (it == input_.GetRoot().AsMap().end())
        throw std::runtime_error("No base_requests field in JSON");

    return it->second;
}

const json::Node& JsonReader::GetStatRequests() const {
    auto it = input_.GetRoot().AsMap().find("stat_requests");
    if (it == input_.GetRoot().AsMap().end())
        throw std::runtime_error("No stat_requests field in JSON");

    return it->second;
}

const json::Node& JsonReader::GetRenderSettings() const {
    auto it = input_.GetRoot().AsMap().find("render_settings");
    if (it == input_.GetRoot().AsMap().end())
        throw std::runtime_error("No render_settings field in JSON");

    return it->second;
}


void JsonReader::ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const {
    json::Array result;

    for (const auto& request_node : stat_requests.AsArray()) {
        const auto& request_map = request_node.AsMap();

        if (!request_map.count("type"))
            throw std::runtime_error("No type field in response node");

        const std::string& type = request_map.at("type").AsString();

        if (type == "Stop")
            result.push_back(PrintStop(request_map, rh).AsMap());
        else if (type == "Bus")
            result.push_back(PrintRoute(request_map, rh).AsMap());
        else if (type == "Map")
            result.push_back(PrintMap(request_map, rh).AsMap());
        else
            throw std::runtime_error("Unknown request type: " + type);
    }

    json::Print(json::Document{ result }, std::cout);
}


void JsonReader::FillCatalogue(transport::TransportCatalogue& catalogue) {
    const json::Array& arr = GetBaseRequests().AsArray();
    for (auto& request_stops : arr) {
        const auto& request_stops_map = request_stops.AsMap();
        const auto& type = request_stops_map.at("type").AsString();
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = ProcessStop(request_stops_map);
            catalogue.AddStop(stop_name, coordinates);
        }
    }
    ProcessStopDistances(catalogue);

    for (auto& request_bus : arr) {
        const auto& request_bus_map = request_bus.AsMap();
        const auto& type = request_bus_map.at("type").AsString();
        if (type == "Bus") {
            auto [bus_number, stops, circular_route] = ProcessRoute(request_bus_map, catalogue);
            catalogue.AddRoute(bus_number, stops, circular_route);
        }
    }
}

// Преобразует словарь Json с информацией о точке остановки в tuple
// Если в словаре отсутствует какой-либо ключ, выбрасывает исключение
std::tuple<std::string_view, geo::Coordinates, std::map<std::string_view, int>> JsonReader::ProcessStop(const json::Dict& stop_request) const {
    std::string_view stop_name = stop_request.at("name").AsString();// название остановки
    geo::Coordinates coordinates = { stop_request.at("latitude").AsDouble(), stop_request.at("longitude").AsDouble() }; // координаты остановки
    std::map<std::string_view, int> stop_distances; // расстояния до ближайших остановок
    auto& distances = stop_request.at("road_distances").AsMap();
    for (auto& [near_stop_name, distance] : distances) {
        stop_distances.emplace(near_stop_name, distance.AsInt());
    }
    return std::make_tuple(stop_name, coordinates, stop_distances);
}


void JsonReader::ProcessStopDistances(transport::TransportCatalogue& catalogue) const {
    const json::Array& arr = GetBaseRequests().AsArray();
    for (auto& request_stops : arr) {
        const auto& request_stops_map = request_stops.AsMap();
        const auto& type = request_stops_map.at("type").AsString();
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = ProcessStop(request_stops_map);
            for (auto& [to_name, dist] : stop_distances) {
                auto from = catalogue.FindStop(stop_name);
                auto to = catalogue.FindStop(to_name);
                catalogue.SetDistance(from, to, dist);
            }
        }
    }
}

// Преобразует словарь Json с информацией о маршруте в tuple
// Если в словаре отсутствует какой-либо ключ, выбрасывает исключение
std::tuple<std::string_view, std::vector<const transport::Stop*>, bool> JsonReader::ProcessRoute(const json::Dict& request_map, transport::TransportCatalogue& catalogue) const {
    std::string_view bus_number = request_map.at("name").AsString(); // номер автобуса
    std::vector<const transport::Stop*> stops;  // остановки на маршруте
    for (auto& stop : request_map.at("stops").AsArray()) {
        stops.push_back(catalogue.FindStop(stop.AsString())); // добавление остановки в вектор
    }
    bool circular_route = request_map.at("is_roundtrip").AsBool(); // является ли маршрут кольцевым

    return std::make_tuple(bus_number, stops, circular_route);
}


renderer::MapRenderer JsonReader::FillRenderSettings(const json::Dict& request_map) const {
    renderer::RenderSettings render_settings;
    render_settings.width = request_map.at("width").AsDouble();
    render_settings.height = request_map.at("height").AsDouble();
    render_settings.padding = request_map.at("padding").AsDouble();
    render_settings.stop_radius = request_map.at("stop_radius").AsDouble();
    render_settings.line_width = request_map.at("line_width").AsDouble();
    render_settings.bus_label_font_size = request_map.at("bus_label_font_size").AsInt();
    const json::Array& bus_label_offset = request_map.at("bus_label_offset").AsArray();
    render_settings.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };
    render_settings.stop_label_font_size = request_map.at("stop_label_font_size").AsInt();
    const json::Array& stop_label_offset = request_map.at("stop_label_offset").AsArray();
    render_settings.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };

    if (request_map.at("underlayer_color").IsString()) render_settings.underlayer_color = request_map.at("underlayer_color").AsString();
    else if (request_map.at("underlayer_color").IsArray()) {
        const json::Array& underlayer_color = request_map.at("underlayer_color").AsArray();
        if (underlayer_color.size() == 3) {
            render_settings.underlayer_color = svg::Rgb(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt());
        }
        else if (underlayer_color.size() == 4) {
            render_settings.underlayer_color = svg::Rgba(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt(), underlayer_color[3].AsDouble());
        }
        else throw std::logic_error("wrong underlayer colortype");
    }
    else throw std::logic_error("wrong underlayer color");

    render_settings.underlayer_width = request_map.at("underlayer_width").AsDouble();

    const json::Array& color_palette = request_map.at("color_palette").AsArray();
    for (const auto& color_element : color_palette) {
        if (color_element.IsString()) render_settings.color_palette.push_back(color_element.AsString());
        else if (color_element.IsArray()) {
            const json::Array& color_type = color_element.AsArray();
            if (color_type.size() == 3) {
                render_settings.color_palette.push_back(svg::Rgb(color_type[0].AsInt(), color_type[1].AsInt(), color_type[2].AsInt()));
            }
            else if (color_type.size() == 4) {
                render_settings.color_palette.push_back(svg::Rgba(color_type[0].AsInt(), color_type[1].AsInt(), color_type[2].AsInt(), color_type[3].AsDouble()));
            }
            else throw std::logic_error("wrong color_palette type");
        }
        else throw std::logic_error("wrong color_palette");
    }

    return render_settings;
}

const json::Node JsonReader::PrintRoute(const json::Dict& request_map, RequestHandler& rh) const {
    json::Dict result;
    const std::string& route_number = request_map.at("name").AsString();
    result["request_id"] = request_map.at("id").AsInt();
    if (!rh.IsBusNumber(route_number)) {
        result["error_message"] = json::Node{ static_cast<std::string>("not found") };
    }
    else {
        result["curvature"] = rh.GetBusStat(route_number)->curvature;
        result["route_length"] = rh.GetBusStat(route_number)->route_length;
        result["stop_count"] = static_cast<int>(rh.GetBusStat(route_number)->stops_count);
        result["unique_stop_count"] = static_cast<int>(rh.GetBusStat(route_number)->unique_stops_count);
    }

    return json::Node{ result };
}

const json::Node JsonReader::PrintStop(const json::Dict& request_map, RequestHandler& rh) const {
    json::Dict result;
    const std::string& stop_name = request_map.at("name").AsString();
    result["request_id"] = request_map.at("id").AsInt();
    if (!rh.IsStopName(stop_name)) {
        result["error_message"] = json::Node{ static_cast<std::string>("not found") };
    }
    else {
        json::Array buses;
        for (auto& bus : rh.GetBusesByStop(stop_name)) {
            buses.push_back(bus);
        }
        result["buses"] = buses;
    }

    return json::Node{ result };
}

const json::Node JsonReader::PrintMap(const json::Dict& request_map, RequestHandler& rh) const {
    json::Dict result;
    result["request_id"] = request_map.at("id").AsInt();
    std::ostringstream strm;
    svg::Document map = rh.RenderMap();
    map.Render(strm);
    result["map"] = strm.str();

    return json::Node{ result };
}
