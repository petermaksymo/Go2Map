/* 
 * Copyright 2018 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "math.h"
#include <algorithm>

struct InfoIntersections {
    std::vector< std::vector<unsigned> > connected_street_segments;
};

struct MapInfo {
    InfoIntersections intersection_db;
    std::vector< std::vector<unsigned> > street_db;
};

MapInfo MAP;

bool load_map(std::string map_path) {
    bool load_successful = loadStreetsDatabaseBIN(map_path);
   
    if(not load_successful) return false;
    
    //Load your map related data structures here
    // The time constraint is 3000ms for load_map
    MAP.street_db.resize(getNumStreets());
    for(int i = 0; i < getNumStreetSegments(); i++) {
        InfoStreetSegment segment = getInfoStreetSegment(i);
        (MAP.street_db[segment.streetID]).push_back(i);
    }
    
    MAP.intersection_db.connected_street_segments.resize(getNumIntersections());
    for(int i = 0; i < getNumIntersections(); i++) {
        for (int j = 0; j < getIntersectionStreetSegmentCount(i); j++ ) {
            MAP.intersection_db.connected_street_segments[i].push_back(getIntersectionStreetSegment(j, i));
        } 
    }
    


    return load_successful;
}

void close_map() {
    //Clean-up your map related data structures here
    closeStreetDatabase();
    
}

////////////////////////////////////////////////
//// Helper functions to be used for m1 ////////
////////////////////////////////////////////////

//sorts and removes all duplicates in a vector
template<typename Type>
void removeDuplicates(std::vector<Type>& vec) {
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}

////////////////////////////////////////////////
////// End of helper functions for m1  /////////
////////////////////////////////////////////////

std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id) {    
    return MAP.intersection_db.connected_street_segments[intersection_id];
}

std::vector<std::string> find_intersection_street_names(unsigned intersection_id) {
    std::vector<std::string> street_names;
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id); i++ ) {
        StreetIndex street_index = getInfoStreetSegment(getIntersectionStreetSegment(i, intersection_id)).streetID;
               
        street_names.push_back(getStreetName(street_index));
    }
    
    return street_names;
}

bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2) {
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id1); i++ ) {
        StreetSegmentIndex street_segment_index = getIntersectionStreetSegment(i, intersection_id1); 
        
        if(unsigned(getInfoStreetSegment(street_segment_index).to) == intersection_id2 
            || unsigned(getInfoStreetSegment(street_segment_index).from) == intersection_id2 ) 
                return true;
    }
    
    return false;
}

std::vector<unsigned> find_adjacent_intersections(unsigned intersection_id) {
    std::vector<unsigned> adjacent_intersections;
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id); i++ ) {
        StreetSegmentIndex street_segment_index = getIntersectionStreetSegment(i, intersection_id); 
        InfoStreetSegment s_segment = getInfoStreetSegment(street_segment_index);
         
        if(s_segment.oneWay) {
            if(unsigned(s_segment.from) == intersection_id)
                adjacent_intersections.push_back(s_segment.to);
        } else
            adjacent_intersections.push_back(
                unsigned(s_segment.to) == intersection_id ? s_segment.from : s_segment.to
            );
    }
    removeDuplicates(adjacent_intersections);
    
    return adjacent_intersections;
}

std::vector<unsigned> find_street_street_segments(unsigned street_id) {
    return MAP.street_db[street_id];
}

std::vector<unsigned> find_all_street_intersections(unsigned street_id) {
    std::vector<unsigned> street = MAP.street_db[street_id];
    std::vector<unsigned> street_intersections;
    
    for(std::vector<unsigned>::iterator it = street.begin(); it != street.end(); it++) {
        street_intersections.push_back(getInfoStreetSegment(*it).from);
        street_intersections.push_back(getInfoStreetSegment(*it).to);
    }
    
    removeDuplicates(street_intersections);
        
    return street_intersections;
}

std::vector<unsigned> find_intersection_ids_from_street_ids(unsigned street_id1, 
                                                              unsigned street_id2) {
    std::vector<unsigned> intersection_ids = {0, 1};
    
    return intersection_ids;
}

double find_distance_between_two_points(LatLon point1, LatLon point2) {
    double avg_lat = (point1.lat() + point2.lat()) / 2;
    double point1_y = point1.lat();
    double point1_x = point1.lon() * cos(avg_lat);
    double point2_y= point2.lat();
    double point2_x= point2.lon() * cos(avg_lat);
    double distance = EARTH_RADIUS_IN_METERS * sqrt(pow((point2_y-point1_y),2) + pow((point2_x-point1_x), 2) );
    return distance;
}

double find_street_segment_length(unsigned street_segment_id) {
    struct InfoStreetSegment street_segment = getInfoStreetSegment(street_segment_id);
    double distance = 0.0;
    int numOfCurves = street_segment.curvePointCount;
    IntersectionIndex point1 = street_segment.from;
    IntersectionIndex point2 = street_segment.to;
    if (numOfCurves == 0) {
        distance = find_distance_between_two_points(getIntersectionPosition(point1), getIntersectionPosition(point2));
    } else {
        for (int i = 0; i < numOfCurves; i++) {
            distance += find_distance_between_two_points(getIntersectionPosition(point1), getIntersectionPosition(point2)); 
        }
    }
    return distance;
}

double find_street_length(unsigned street_id) {
    return 0.0;
}

double find_street_segment_travel_time(unsigned street_segment_id) {
    return 0.0;
}

unsigned find_closest_point_of_interest(LatLon my_position) {
    int min_distance = -1;
    POIIndex min_index = 0;
    int distance_temp = 0;
    // loop over every intersection, calculating distance between each intersection and my_position
    for(int i = 0; i < getNumPointsOfInterest(); i++) {
        distance_temp =  find_distance_between_two_points(my_position, getPointOfInterestPosition(i));
        // keep track of minimum distance between points
        if(distance_temp < min_distance || min_distance == -1) {
            min_index = i;
            min_distance = distance_temp;
        }
    }
    return min_index;
}

unsigned find_closest_intersection(LatLon my_position) {
    int min_distance = -1;
    IntersectionIndex min_index = 0;
    int distance_temp = 0;
    // loop over every intersection, calculating distance between each intersection and my_position
    for(int i = 0; i < getNumIntersections(); i++) {
        distance_temp =  find_distance_between_two_points(my_position, getIntersectionPosition(i));
        // keep track of minimum distance between points
        if(distance_temp < min_distance || min_distance == -1) {
            min_index = i;
            min_distance = distance_temp;
        }
    }
    return min_index;
}

std::vector<unsigned> find_street_ids_from_partial_street_name(std::string street_prefix) {
    std::vector<unsigned> street_ids = {0, 1};
    
    return street_ids;
}