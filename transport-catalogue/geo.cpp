#define _USE_MATH_DEFINES
#include "geo.h"
 
#include <cmath>
 
namespace geo {
    
double compute_distance(Coordinates start, Coordinates end) {
    using namespace std;
    
    if (!(start == end)) {
        const double dr = M_PI / 180.;
        return acos(sin(start.latitude * dr) * sin(end.latitude * dr)
                    + cos(start.latitude * dr) * cos(end.latitude * dr) 
                    * cos(abs(start.longitude - end.longitude) * dr)) * 6371000;
    } else {
        return 0.0;
    }
}
    
}//end namespace geo
