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
#include "map_db.h"
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "OSMFetchers.h"
#include "math.h"
#include <algorithm>
#include <map>
#include <boost/algorithm/string.hpp>
#include "helper_functions.h"

bool load_map(std::string map_path) {
    //get the path to the osm.bin file by replacing the extension of map_path
    std::string OSM_map_path = map_path;
    std::string to_replace = "streets.bin";
    OSM_map_path.replace(OSM_map_path.rfind(to_replace), to_replace.length(), "osm.bin");
    
    bool load_successful = loadStreetsDatabaseBIN(map_path) &&
                           loadOSMDatabaseBIN(OSM_map_path);
   
    if(not load_successful) return false;
    
    display_osm_info();
    
    //Load our map related data structures here
    load_street_segments();
    
    load_intersections();

    load_streets();
    
    return load_successful;
}


void close_map() {
    MAP.street_name_id_map.clear();
    MAP.intersection_db.clear();
    MAP.street_db.clear();
    
    closeOSMDatabase();
    closeStreetDatabase();
}

//Returns the street segments for the given intersection 
//pre-computed in load_map for performance
std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id) {    
    return MAP.intersection_db[intersection_id].connected_street_segments;
}


//Returns all the street names at the given intersection
std::vector<std::string> find_intersection_street_names(unsigned intersection_id) {
    std::vector<std::string> street_names;
    
    //goes through all street segments in intersection and adds name to street_names
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id); i++ ) {
        StreetIndex street_index = getInfoStreetSegment(getIntersectionStreetSegment(i, intersection_id)).streetID;      
        street_names.push_back(getStreetName(street_index));
    }
    
    return street_names;
}


//Returns true if you can get from intersection1 to intersection2 using a single street segment
bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2) {
    //catch intersection connected to itself corner case
    if (intersection_id1 == intersection_id2) return true;
    
    //goes through all connected street segments to intersection 1
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id1); i++ ) {
        StreetSegmentIndex street_segment_index = getIntersectionStreetSegment(i, intersection_id1); 
        InfoStreetSegment street_segment = getInfoStreetSegment(street_segment_index);
        
        if (street_segment.oneWay) {
            //check one way special case
            if(unsigned(street_segment.to) == intersection_id2 )
                return true;
        } else {
            //checks if the segment is connected to intersection 2
            if(unsigned(street_segment.to) == intersection_id2 
                || unsigned(street_segment.from) == intersection_id2 ) 
                    return true;
        }
    }
    
    return false;
}


//Returns all intersections reachable by traveling down one street segment 
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
            //adds the intersection that isn't the one called
            adjacent_intersections.push_back(
                unsigned(s_segment.to) == intersection_id ? s_segment.from : s_segment.to
            );
        }
    }
    removeDuplicates(adjacent_intersections);
    
    return adjacent_intersections;
}


//Returns all street segments for the given street
//pre-computed in load_map for performance
std::vector<unsigned> find_street_street_segments(unsigned street_id) {
    return MAP.street_db[street_id].segments;
}


//Returns all intersections along the a given street
//pre-computed in load_map for performance
std::vector<unsigned> find_all_street_intersections(unsigned street_id) {        
    return MAP.street_db[street_id].intersections;
}


//Returns all intersection ids between two streets
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


// Calculate distance between two points in LatLon form base on average Latitude
double find_distance_between_two_points(LatLon point1, LatLon point2) {
    double avg_lat = (point1.lat() + point2.lat()) / 2.0 * DEG_TO_RAD; //Average latitude
    // Convert Latitude/Longitude to Cartesian coordinate
    double point1_y = point1.lat() * DEG_TO_RAD;
    double point1_x = point1.lon() * cos(avg_lat) * DEG_TO_RAD;
    
    double point2_y= point2.lat() * DEG_TO_RAD;
    double point2_x= point2.lon() * cos(avg_lat) * DEG_TO_RAD;
    
    double distance = EARTH_RADIUS_IN_METERS * sqrt(pow((point2_y-point1_y),2) + pow((point2_x-point1_x), 2) ); //Distance base on formula provided
    return distance;
}


// Access street segment length pre-computed and stored to MAP
double find_street_segment_length(unsigned street_segment_id) {
    return MAP.LocalStreetSegments.street_segment_length[street_segment_id];
}


// Calculate length of entire street base on each one of its segment
double find_street_length(unsigned street_id) {
    double distance = 0.0;
    // Iterate through each segment of the street
    for (unsigned int i = 0; i < MAP.street_db[street_id].segments.size(); i++) {
        distance = distance + MAP.LocalStreetSegments.street_segment_length[MAP.street_db[street_id].segments[i]];
    }
    return distance;
}


// Calculate travel time by accessing pre-computed street segment length and stored street segment speed limit
double find_street_segment_travel_time(unsigned street_segment_id) {
    double time = 0.0;
    time = MAP.LocalStreetSegments.street_segment_length[street_segment_id] 
            / MAP.LocalStreetSegments.street_segment_speed_limit[street_segment_id] * 3.6; // Multiplying by 3.6 to convert from KM/h to M/s
    return time;
}


// Returns the id to the POI that is closest to the position that is passed
unsigned find_closest_point_of_interest(LatLon my_position) {
    double min_distance = -1;
    POIIndex min_index = 0;
    double distance_temp = 0;
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
    double min_distance = -1;
    IntersectionIndex min_index = 0;
    double distance_temp = 0;
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
    std::multimap<std::string, int>::iterator it;
    
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
