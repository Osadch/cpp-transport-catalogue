#pragma once


#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <iostream>

struct RouteInfo {
    std::string_view number;
    std::vector<const transport::Stop*> stops;
    bool is_circular;
};

struct StopInfo {
    std::string_view name;
    geo::Coordinates coordinates;
    std::map<std::string_view, int> distances;
};

enum ColorType {
    RGB = 3,
    RGBA = 4
};

class JsonReader {
public:
    JsonReader(std::istream& input)
        : input_(json::Load(input))
    {}

    const json::Node& GetBaseRequests() const;
    const json::Node& GetStatRequests() const;
    const json::Node& GetRenderSettings() const;

    void ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const;

    void FillCatalogue(transport::TransportCatalogue& catalogue);
    renderer::MapRenderer FillRenderSettings(const json::Dict& request_map) const;

    

    const json::Node PrintRoute(const json::Dict& request_map, RequestHandler& rh) const;
    const json::Node PrintStop(const json::Dict& request_map, RequestHandler& rh) const;
    const json::Node PrintMap(const json::Dict& request_map, RequestHandler& rh) const;
    RouteInfo ProcessRoute(const json::Dict& request_map, transport::TransportCatalogue& catalogue) const;
    StopInfo ProcessStop(const json::Dict& stop_request) const;

private:
    json::Document input_;
    json::Node dummy_ = nullptr;
    
    void ProcessStopDistances(transport::TransportCatalogue& catalogue) const;  
    
};
