#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <transport_catalogue.pb.h>

void SerializeCatalogue(const transport::Catalogue& catalogue,
    const renderer::MapRenderer& renderer,
    const transport::Router& router,
    std::ostream& output);

serialize::Stop SerializeStop(const transport::Stop* stop);

serialize::Bus SerializeBus(const transport::Bus* bus);

serialize::RenderSettings GetSerializeRenderSettings(const json::Node& render_settings);

serialize::RouterSettings GetSerializeRouterSettings(const json::Node& router_settings);

serialize::Router SerializeRouter(const transport::Router& router);

std::tuple<transport::Catalogue, renderer::MapRenderer, transport::Router,
    graph::DirectedWeightedGraph<double>, std::map<std::string, graph::VertexId>> Deserialize(std::istream& input);
