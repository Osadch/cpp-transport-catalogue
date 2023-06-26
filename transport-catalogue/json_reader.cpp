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
    for (const auto& request : arr) {
        const auto& request_map = request.AsMap();
        const auto& type = request_map.at("type").AsString();
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = ProcessStop(request_map);
            catalogue.AddStop(stop_name, coordinates);
        }
        if (type == "Bus") {
            auto [bus_number, stops, circular_route] = ProcessRoute(request_map, catalogue);
            catalogue.AddRoute(bus_number, stops, circular_route);
        }
    }
    ProcessStopDistances(catalogue);
}

//

StopInfo JsonReader::ProcessStop(const json::Dict& stop_request) const {
    StopInfo stop_info;
    stop_info.name = stop_request.at("name").AsString();
    stop_info.coordinates = { stop_request.at("latitude").AsDouble(), stop_request.at("longitude").AsDouble() };
    auto& distances = stop_request.at("road_distances").AsMap();
    for (auto& [near_stop_name, distance] : distances) {
        stop_info.distances.emplace(near_stop_name, distance.AsInt());
    }
    return stop_info;
}


void JsonReader::ProcessStopDistances(transport::TransportCatalogue& catalogue) const {
    const json::Array& arr = GetBaseRequests().AsArray();
    for (const auto& request_stops : arr) {
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

//

RouteInfo JsonReader::ProcessRoute(const json::Dict& request_map, transport::TransportCatalogue& catalogue) const {
    RouteInfo route_info;
    route_info.number = request_map.at("name").AsString();
    for (auto& stop : request_map.at("stops").AsArray()) {
        route_info.stops.push_back(catalogue.FindStop(stop.AsString()));
    }
    route_info.is_circular = request_map.at("is_roundtrip").AsBool();
    return route_info;
}


const auto MAX_COLOR_TYPE = static_cast<int>(ColorType::RGBA);


svg::Color ParseColor(const json::Node& color_node) {
    if (color_node.IsString()) {
        return color_node.AsString();
    }
    else if (color_node.IsArray()) {
        auto color_array = color_node.AsArray();
        if (color_array.size() == ColorType::RGB) {
            return svg::Rgb(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt());
        }
        else if (color_array.size() == ColorType::RGBA) {
            return svg::Rgba(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble());
        }
        else {
            throw std::logic_error("wrong color type");
        }
    }
    else {
        throw std::logic_error("wrong color type");
    }
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


    auto underlayer_color_node = request_map.at("underlayer_color");
    if (underlayer_color_node.IsString()) {
        render_settings.underlayer_color = underlayer_color_node.AsString();
    }
    else if (underlayer_color_node.IsArray()) {
        auto underlayer_color_arr = underlayer_color_node.AsArray();
        auto underlayer_color_size = underlayer_color_arr.size();
        if (underlayer_color_size == ColorType::RGB) {
            render_settings.underlayer_color = svg::Rgb{
 static_cast<uint8_t>(underlayer_color_arr[0].AsInt()),
 static_cast<uint8_t>(underlayer_color_arr[1].AsInt()),
 static_cast<uint8_t>(underlayer_color_arr[2].AsInt())
            };
        }
        else if (underlayer_color_size == ColorType::RGBA) {
            render_settings.underlayer_color = svg::Rgba{
    static_cast<uint8_t>(underlayer_color_arr[0].AsInt()),
    static_cast<uint8_t>(underlayer_color_arr[1].AsInt()),
    static_cast<uint8_t>(underlayer_color_arr[2].AsInt()),
    static_cast<double>(underlayer_color_arr[3].AsDouble())
            };
        }
        else {
            throw std::logic_error("wrong underlayer color");
        }
    }
    else {
        throw std::logic_error("wrong underlayer color");
    }

    render_settings.underlayer_width = request_map.at("underlayer_width").AsDouble();
    const json::Array& color_palette = request_map.at("color_palette").AsArray();
    for (const auto& color_element : color_palette) {
        auto parsed_color = ParseColor(color_element);
        render_settings.color_palette.push_back(parsed_color);
    }

    return renderer::MapRenderer(render_settings);
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
