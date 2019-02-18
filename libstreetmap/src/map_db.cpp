/*
 * Declares the global MAP object
 * Contains the helper functions used to populate the MAP object
 * that are called in load_map
 */

#include "map_db.h"
#include "StreetsDatabaseAPI.h"
#include <map>
#include <boost/algorithm/string.hpp>
#include "helper_functions.h"

//define the global MAP object
MapInfo MAP;


//Helper functions called in load_map:

// load extra street segment related info into MAP
void load_street_segments () {
   // Resize for tiny performance benefit
    MAP.street_db.resize(getNumStreets());
    
    //Iterating through all street segments
    //Loads up street_db with all segments of a street, and calculate lengths
    for(int i = 0; i < getNumStreetSegments(); i++) {
        InfoStreetSegment segment = getInfoStreetSegment(i);
        
        MAP.street_db[segment.streetID].segments.push_back(i);
        
        MAP.LocalStreetSegments.street_segment_length.push_back(street_segment_length_helper(i));
        MAP.LocalStreetSegments.street_segment_speed_limit.push_back(segment.speedLimit);
    } 
}

// load extra intersection info into MAP
// Also determines the max and min LatLon for drawing
void load_intersections () {
    MAP.intersection_db.resize(getNumIntersections());
    
    //initialize world values so comparing later works
    MAP.world_values.max_lat = getIntersectionPosition(0).lat();
    MAP.world_values.min_lat = MAP.world_values.max_lat;
    MAP.world_values.max_lon = getIntersectionPosition(0).lon();
    MAP.world_values.min_lon = MAP.world_values.max_lon;
    
    //Iterating through all intersections
    //Loads up intersections_db with all connected street segments
    for(int i = 0; i < getNumIntersections(); i++) {
        for (int j = 0; j < getIntersectionStreetSegmentCount(i); j++ ) {
            MAP.intersection_db[i].connected_street_segments.push_back(getIntersectionStreetSegment(j, i));
        }         
        MAP.intersection_db[i].position = getIntersectionPosition(i);
        MAP.intersection_db[i].name = getIntersectionName(i);
        
        //Check and update min/max lat/lon in world_values
        MAP.world_values.max_lat = std::max(MAP.world_values.max_lat, MAP.intersection_db[i].position.lat());
        MAP.world_values.min_lat = std::min(MAP.world_values.min_lat, MAP.intersection_db[i].position.lat());
        MAP.world_values.max_lon = std::max(MAP.world_values.max_lon, MAP.intersection_db[i].position.lon());
        MAP.world_values.min_lon = std::min(MAP.world_values.min_lon, MAP.intersection_db[i].position.lon());
    }
}

// loads street data into MAP
void load_streets () { 
    //Iterating through all streets 
    for(unsigned i = 0; i < MAP.street_db.size(); i++) {
        
        //Loads up street_db with all intersections of a street
        for(std::vector<unsigned>::iterator it = MAP.street_db[i].segments.begin(); 
                it != MAP.street_db[i].segments.end(); it++) {
            
            MAP.street_db[i].intersections.push_back(getInfoStreetSegment(*it).from);
            MAP.street_db[i].intersections.push_back(getInfoStreetSegment(*it).to);
        }
        removeDuplicates(MAP.street_db[i].intersections);
        
        //Load street_name_id_map with all street name and id pairs
        std::string street_name = getStreetName(i);
        boost::algorithm::to_lower(street_name);
        
        // insert pair into multimap - multimap allows for duplicate keys
        MAP.street_name_id_map.insert(std::pair <std::string, int> (street_name, i));
    }
}
