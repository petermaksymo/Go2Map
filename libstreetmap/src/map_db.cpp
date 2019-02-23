/*
 * Declares the global MAP object
 * Contains the helper functions used to populate the MAP object
 * that are called in load_map
 */

#include "map_db.h"
#include "KD2Tree.h"
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
    
    std::vector<std::pair<std::pair<double, double>, unsigned int>> street_segs_zoom_0;
    std::vector<std::pair<std::pair<double, double>, unsigned int>> street_segs_zoom_1;
    
    //Iterating through all street segments
    //Loads up street_db with all segments of a street, and calculate lengths
    for(int i = 0; i < getNumStreetSegments(); i++) {
        InfoStreetSegment segment = getInfoStreetSegment(i);
        
        MAP.street_db[segment.streetID].segments.push_back(i);
        
        MAP.LocalStreetSegments.street_segment_length.push_back(street_segment_length_helper(i));
        MAP.LocalStreetSegments.street_segment_speed_limit.push_back(segment.speedLimit);
        
        // Add to/from positions to street segs vector for appropriate zoom level in KD2Tree
        std::pair<double, double> f_point = std::make_pair(x_from_lon(MAP.intersection_db[segment.from].position.lon()), y_from_lat(MAP.intersection_db[segment.from].position.lat()));
        std::pair<std::pair<double, double>, unsigned int> from_pt = std::make_pair(f_point, i);
        
        std::pair<double, double> t_point = std::make_pair(x_from_lon(MAP.intersection_db[segment.to].position.lon()), y_from_lat(MAP.intersection_db[segment.to].position.lat()));
        std::pair<std::pair<double, double>, unsigned int> to_pt = std::make_pair(t_point, i);
        
        if(segment.speedLimit >= 90) {
            street_segs_zoom_0.push_back(from_pt);
            street_segs_zoom_0.push_back(to_pt);
        } else {
            street_segs_zoom_1.push_back(from_pt);
            street_segs_zoom_1.push_back(to_pt);
        }
    }
    
    
    // Load both zoom levels to tree
    MAP.street_seg_k2tree.root = MAP.street_seg_k2tree.make_tree(street_segs_zoom_0.begin(), street_segs_zoom_0.end(), 0, street_segs_zoom_0.size(), 0);
    MAP.street_seg_k2tree.insert_bulk(street_segs_zoom_1.begin(), street_segs_zoom_1.end(), MAP.street_seg_k2tree.root, 0, street_segs_zoom_1.size(), 1);
    
    street_segs_zoom_0.clear();
    street_segs_zoom_1.clear();
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
    
    //initialize out of bounds
    MAP.state.last_selected_intersection = getNumIntersections() + 1;
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
