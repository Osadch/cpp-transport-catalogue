#pragma once

#include <cmath>
 
namespace geo {
    
struct Coordinates {
    double latitude; // Широта
    double longitude; // Долгота
 
    bool operator==(const Coordinates& other) const {
        return latitude == other.latitude && longitude == other.longitude;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};    
double compute_distance(Coordinates start, Coordinates end);
    
}//end namespace geo
