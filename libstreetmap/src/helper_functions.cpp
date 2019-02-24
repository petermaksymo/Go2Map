/* Useful helper functions */

#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "helper_functions.h"
#include "map_db.h"
#include "math.h"
#include <algorithm>
#include <ezgl/point.hpp>

//sorts and removes all duplicates in a vector
void removeDuplicates(std::vector<unsigned>& vec) {
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}

double y_from_lat (double lat) {  
    return (lat * DEG_TO_RAD);
}

double x_from_lon (double lon) {
    double avg_lat = (MAP.world_values.max_lat + MAP.world_values.min_lat) / 2.0 * DEG_TO_RAD;
    
    return (lon * cos(avg_lat) * DEG_TO_RAD);
}

double lat_from_y (double y) {
    return (y / DEG_TO_RAD);
}

double lon_from_x (double x) {
    double avg_lat = (MAP.world_values.max_lat + MAP.world_values.min_lat) / 2.0 * DEG_TO_RAD;
    
    return (x / (cos(avg_lat) * DEG_TO_RAD));
}

ezgl::point2d point2d_from_LatLon (LatLon point) {
    ezgl::point2d coord(x_from_lon(point.lon()), y_from_lat(point.lat()));
    
    return coord;
}

//Pre-compute length of street segment
double street_segment_length_helper(unsigned street_segment_id) {
    struct InfoStreetSegment street_segment = getInfoStreetSegment(street_segment_id);
    double distance = 0.0;
    int numOfCurves = street_segment.curvePointCount;
    
    LatLon point1 = getIntersectionPosition(street_segment.from);
    LatLon point2 = getIntersectionPosition(street_segment.to);
    
    // In the case that street is straight 
    if (numOfCurves == 0) {
        distance = find_distance_between_two_points(point1, point2);
    } 
    
    // In the case that street has 1 curve point
    else if (numOfCurves == 1) {
        // Calculate distance from two end points to the curve point
        distance = find_distance_between_two_points(point1,getStreetSegmentCurvePoint(0, street_segment_id))
        + find_distance_between_two_points(getStreetSegmentCurvePoint(0, street_segment_id), point2);          
    } 
    
    // In the case that street has more than 1 curve point
    else if (numOfCurves > 1) {
        // Calculate distance between curve points
        for (int i = 0; i < numOfCurves - 1; i++) {
            distance = distance + 
            find_distance_between_two_points(getStreetSegmentCurvePoint(i, street_segment_id),
            getStreetSegmentCurvePoint(i+1, street_segment_id));
        }
        
        // Adding distance of two end points to their adjacent curve points
        distance = distance + 
        find_distance_between_two_points(point1,getStreetSegmentCurvePoint(0, street_segment_id))
        + find_distance_between_two_points(getStreetSegmentCurvePoint(numOfCurves - 1, street_segment_id), point2);
    }
    return distance;
}

double distance_from_points(double x1, double y1, double x2, double y2) {
    return sqrt(pow((x2-x1), 2) + pow((y2-y1), 2));
}
