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
#include <map>
#include <boost/algorithm/string.hpp>
#include "helper_functions.h"

////////////////////////////////////////////////
///// Structures for the MAP database  /////////
////////////////////////////////////////////////

struct InfoIntersections {
    std::vector<unsigned> connected_street_segments;
};

struct InfoStreets {
    std::vector<unsigned> segments;
    std::vector<unsigned> intersections;
};

struct InfoStreetSegmentsLocal {
    std::vector<double> street_segment_length;
    std::vector<double> street_segment_speed_limit; 
};

// The vectors contain are all streets/intersections
struct MapInfo {
    std::vector<InfoIntersections> intersection_db;
    std::vector<InfoStreets> street_db;
    std::map<std::string, int> street_name_id_map;
    InfoStreetSegmentsLocal LocalStreetSegments;
};
////////////////////////////////////////////////
/// END of Structures for the MAP database  ////
////////////////////////////////////////////////

MapInfo MAP;

bool load_map(std::string map_path) {
    bool load_successful = loadStreetsDatabaseBIN(map_path);
   
    if(not load_successful) return false;
    
    //Load your map related data structures here
    // The time constraint is 3000ms for load_map
    
    // Warnign: Other code in load_map depend on street_db having size getNumStreets()
    MAP.street_db.resize(getNumStreets());
    
    //Iterating through all street segments
    //Loads up street_db with all segments of a street
    for(int i = 0; i < getNumStreetSegments(); i++) {
        InfoStreetSegment segment = getInfoStreetSegment(i);
        MAP.street_db[segment.streetID].segments.push_back(i);
        MAP.LocalStreetSegments.street_segment_length.push_back(street_segment_length_helper(i));
        MAP.LocalStreetSegments.street_segment_speed_limit.push_back(segment.speedLimit);
    }
    
    MAP.intersection_db.resize(getNumIntersections());
    
    //Iterating through all intersections
    //Loads up intersections_db with all connected street segments
    for(int i = 0; i < getNumIntersections(); i++) {
        for (int j = 0; j < getIntersectionStreetSegmentCount(i); j++ ) {
            MAP.intersection_db[i].connected_street_segments.push_back(getIntersectionStreetSegment(j, i));
        } 
    }

    //Iterating through all streets
    //Loads up street_db with all intersections of a street 
    for(unsigned i = 0; i < MAP.street_db.size(); i++) {
        for(std::vector<unsigned>::iterator it = MAP.street_db[i].segments.begin(); 
                it != MAP.street_db[i].segments.end(); it++) {
            MAP.street_db[i].intersections.push_back(getInfoStreetSegment(*it).from);
            MAP.street_db[i].intersections.push_back(getInfoStreetSegment(*it).to);
        }
        removeDuplicates(MAP.street_db[i].intersections);
    }
    
    //Iterate through all streets
    //Load street_name_id_map with all street name and id pairs
    std::string street_name = "";
    for(unsigned street_index = 0; street_index < MAP.street_db.size(); street_index++) {
        street_name = getStreetName(street_index);
        boost::algorithm::to_lower(street_name);
        // check if street name already in map, generate continually add a character until is unique
        while(MAP.street_name_id_map.find(street_name) != MAP.street_name_id_map.end()) {
            street_name = street_name + " *";
        }
        MAP.street_name_id_map[street_name] = street_index;
    }
    return load_successful;
}

void close_map() {
    MAP.street_name_id_map.clear();
    MAP.intersection_db.clear();
    MAP.street_db.clear();
    //Clean-up your map related data structures here
    closeStreetDatabase();
    
}


//was pre-computed in load_map for speed
std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id) {    
    return MAP.intersection_db[intersection_id].connected_street_segments;
}


std::vector<std::string> find_intersection_street_names(unsigned intersection_id) {
    std::vector<std::string> street_names;
    
    //goes through all street segments in intersection and adds name to street_names
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id); i++ ) {
        StreetIndex street_index = getInfoStreetSegment(getIntersectionStreetSegment(i, intersection_id)).streetID;      
        street_names.push_back(getStreetName(street_index));
    }
    
    return street_names;
}

bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2) {
    //goes through all connected street segments to intersection 1
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id1); i++ ) {
        StreetSegmentIndex street_segment_index = getIntersectionStreetSegment(i, intersection_id1); 
        
        //checks if the segment is connected to intersection 2
        if(unsigned(getInfoStreetSegment(street_segment_index).to) == intersection_id2 
            || unsigned(getInfoStreetSegment(street_segment_index).from) == intersection_id2 ) 
                return true;
    }
    
    return false;
}

std::vector<unsigned> find_adjacent_intersections(unsigned intersection_id) {
    std::vector<unsigned> adjacent_intersections;
    
    //goes through each connected street segment
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id); i++ ) {
        InfoStreetSegment s_segment = getInfoStreetSegment(getIntersectionStreetSegment(i, intersection_id));
         
        //checks for one way corner case
        if(s_segment.oneWay) {
            if(unsigned(s_segment.from) == intersection_id)
                adjacent_intersections.push_back(s_segment.to);
        } else {
            //adds the intersection that isn't the one called in this function
            adjacent_intersections.push_back(
                unsigned(s_segment.to) == intersection_id ? s_segment.from : s_segment.to
            );
        }
    }
    removeDuplicates(adjacent_intersections);
    
    return adjacent_intersections;
}

//pre-computed in load_map for performance
std::vector<unsigned> find_street_street_segments(unsigned street_id) {
    return MAP.street_db[street_id].segments;
}

//pre-computed in load_map for performance
std::vector<unsigned> find_all_street_intersections(unsigned street_id) {        
    return MAP.street_db[street_id].intersections;
}

std::vector<unsigned> find_intersection_ids_from_street_ids(unsigned street_id1, 
                                                              unsigned street_id2) {
    std::vector<unsigned> intersections;
    
    //goes through all intersections in a street
    for (std::vector<unsigned>::iterator itx = MAP.street_db[street_id1].intersections.begin(); 
            itx != MAP.street_db[street_id1].intersections.end(); itx++) {
        
        //goes through all intersections in the other street
        for (std::vector<unsigned>::iterator ity = MAP.street_db[street_id2].intersections.begin(); 
                ity != MAP.street_db[street_id2].intersections.end(); ity++) {
            
            if(*itx == *ity) intersections.push_back(*itx);
        }
    }
    
    return intersections;
}

double find_distance_between_two_points(LatLon point1, LatLon point2) {
    double avg_lat = (point1.lat() + point2.lat()) / 2.0 * DEG_TO_RAD;
    double point1_y = point1.lat() * DEG_TO_RAD;
    double point1_x = point1.lon() * cos(avg_lat) * DEG_TO_RAD;
    double point2_y= point2.lat() * DEG_TO_RAD;
    double point2_x= point2.lon() * cos(avg_lat) * DEG_TO_RAD;
    double distance = EARTH_RADIUS_IN_METERS * sqrt(pow((point2_y-point1_y),2) + pow((point2_x-point1_x), 2) );
    return distance;
}

double find_street_segment_length(unsigned street_segment_id) {
    return MAP.LocalStreetSegments.street_segment_length[street_segment_id];
}

double find_street_length(unsigned street_id) {
    double distance = 0.0;
    for (unsigned int i = 0; i < MAP.street_db[street_id].segments.size(); i++) {
        distance = distance + MAP.LocalStreetSegments.street_segment_length[MAP.street_db[street_id].segments[i]];
    }
    return distance;
}

double find_street_segment_travel_time(unsigned street_segment_id) {
    double time = 0.0;
    time = MAP.LocalStreetSegments.street_segment_length[street_segment_id] 
            / MAP.LocalStreetSegments.street_segment_speed_limit[street_segment_id] * 3.6;
    return time;
}

// Returns the id to the POI that is closest to the position that is passed
unsigned find_closest_point_of_interest(LatLon my_position) {
    int min_distance = -1;
    POIIndex min_index = 0;
    int distance_temp = 0;
    // loop over every point of interest, calculating distance between each point of interest and my_position
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


// Returns the id to the intersection that is closest to the position that is passed
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


// Finds street ids from the prefix of a street e.g. "bloO" finds id of "Bloor Street"
// Street name and id pairs are pre-computed and stored in a map
// Uses the lower_bound function to find the spot that the given prefix could be inserted to the list
// Continues check the next street name to see if it is a prefix, returns once not a prefix
std::vector<unsigned> find_street_ids_from_partial_street_name(std::string street_prefix) {
    std::vector<unsigned> street_ids = {};
    std::map<std::string, int>::iterator it;
    
    boost::algorithm::to_lower(street_prefix);
    
    // return empty vector if passed empty string
    if(street_prefix.length() == 0) return street_ids;
    
    // lower_bound finds the first key in the map that the street_prefix should go before or be equal to
    it = MAP.street_name_id_map.lower_bound(street_prefix); // < O(logN)
    
    // if the it is not the end of the map AND
    // if the street name has the prefix
    while(it != MAP.street_name_id_map.end() && (it->first).rfind(street_prefix, 0) == 0) {
        // add the id and check the next in the map
        street_ids.push_back(it->second);
        it++;
    }

    return street_ids;
}
