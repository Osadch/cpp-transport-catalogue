#pragma once
 
#include <algorithm>
#include <vector>
#include <string>
#include <variant>
 
#include "geo.h"
#include "graph.h"

using namespace std;
 
namespace domain { 
    
struct Stop;
 
struct StatRequest { 
    int id = 0;
    string type;
    string name;    
    string from;
    string to;
};
    
 struct Bus { 
    string name;
    vector<Stop*> stops;
    bool is_roundtrip;
    size_t route_length;
};   

struct Stop {    
    string name;
    double latitude = 0.0;
    double longitude = 0.0;    
    vector<Bus*> buses;
}; 

struct Distance {    
    const Stop* start;
    const Stop* end;
    int distance = 0;
};  

struct BusQueryResult {
    string_view name;
    bool not_found;
    int stops_on_route = 0;
    int unique_stops = 0;
    int route_length = 0;
    double curvature = 0.0;
};   
 
struct StopQueryResult {
    string_view name;
    bool not_found;
    vector<string> buses_name;
};    

struct StopEdge {
    string_view name;
    double time = 0.0;
};

struct BusEdge {
    string_view bus_name;
    size_t span_count = 0;
    double time = 0.0;
};
 
struct RoutingSettings {
    double bus_wait_time = 0.0;
    double bus_velocity = 0.0;
};
 
struct RouterByStop {
    graph::VertexId bus_wait_start;
    graph::VertexId bus_wait_end;
};
 
struct RouteInfo {
    double total_time = 0.0;
    vector<variant<StopEdge, BusEdge>> edges;
};
 }
