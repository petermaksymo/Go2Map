/*
 * Declares the global MAP object
 * Contains the helper functions used to populate the MAP object
 * that are called in load_map
 */

#include "map_db.h"
#include "constants.hpp"
#include "KD2Tree.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
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
    
    std::vector<std::pair<std::pair<double, double>, unsigned int>> street_segs_zoom_m1;
    std::vector<std::pair<std::pair<double, double>, unsigned int>> street_segs_zoom_0;
    std::vector<std::pair<std::pair<double, double>, unsigned int>> street_segs_zoom_1;
    std::vector<std::pair<std::pair<double, double>, unsigned int>> street_segs_zoom_2;
    
    //Iterating through all street segments
    //Loads up street_db with all segments of a street, and calculate lengths
    for(int i = 0; i < getNumStreetSegments(); i++) {
        InfoStreetSegment segment = getInfoStreetSegment(i);
        
        MAP.street_db[segment.streetID].segments.push_back(i);
        
        InfoStreetSegmentsLocal to_load;
        to_load.street_segment_length = street_segment_length_helper(i);
        to_load.street_segment_speed_limit = segment.speedLimit;
        to_load.travel_time = to_load.street_segment_length / (to_load.street_segment_speed_limit / 3.6);
        to_load.importance_level = get_street_segment_importance(i);
        MAP.LocalStreetSegments.push_back(to_load);
        
        //  Calculate length of street and average speed limit
        MAP.street_db[segment.streetID].length += MAP.LocalStreetSegments[i].street_segment_length;
        MAP.street_db[segment.streetID].average_speed = (MAP.street_db[segment.streetID].average_speed*MAP.street_db[segment.streetID].segments.size() +
                                                           + MAP.LocalStreetSegments[i].street_segment_speed_limit)/(MAP.street_db[segment.streetID].segments.size() + 1);

        // Add to/from positions to street segs vector for appropriate zoom level in KD2Tree
        std::pair<double, double> f_point = std::make_pair(x_from_lon(MAP.intersection_db[segment.from].position.lon()), y_from_lat(MAP.intersection_db[segment.from].position.lat()));
        std::pair<std::pair<double, double>, unsigned int> from_pt = std::make_pair(f_point, i);
        
        std::pair<double, double> t_point = std::make_pair(x_from_lon(MAP.intersection_db[segment.to].position.lon()), y_from_lat(MAP.intersection_db[segment.to].position.lat()));
        std::pair<std::pair<double, double>, unsigned int> to_pt = std::make_pair(t_point, i);
        
        
        if(MAP.LocalStreetSegments[i].importance_level < 0) {
            street_segs_zoom_m1.push_back(from_pt);
            street_segs_zoom_m1.push_back(to_pt);
        } else if(MAP.LocalStreetSegments[i].importance_level <= 2) {
            street_segs_zoom_0.push_back(from_pt);
            street_segs_zoom_0.push_back(to_pt);
        } else if(MAP.LocalStreetSegments[i].importance_level <=3 && MAP.street_db[segment.streetID].length > 200 && MAP.street_db[segment.streetID].length < 1000000) {
            street_segs_zoom_1.push_back(from_pt);
            street_segs_zoom_1.push_back(to_pt);
        } else {
            street_segs_zoom_2.push_back(from_pt);
            street_segs_zoom_2.push_back(to_pt);
        }
    }
    
    // Load both zoom levels to tree
    MAP.street_seg_k2tree.root = MAP.street_seg_k2tree.make_tree(street_segs_zoom_m1.begin(), street_segs_zoom_m1.end(), 0, street_segs_zoom_m1.size(), -1);
    MAP.street_seg_k2tree.insert_bulk(street_segs_zoom_0.begin(), street_segs_zoom_0.end(), MAP.street_seg_k2tree.root, 0, street_segs_zoom_0.size(), 0);
    MAP.street_seg_k2tree.insert_bulk(street_segs_zoom_1.begin(), street_segs_zoom_1.end(), MAP.street_seg_k2tree.root, 0, street_segs_zoom_1.size(), 1);
    MAP.street_seg_k2tree.insert_bulk(street_segs_zoom_2.begin(), street_segs_zoom_2.end(), MAP.street_seg_k2tree.root, 0, street_segs_zoom_2.size(), 2);
         
    street_segs_zoom_m1.clear();
    street_segs_zoom_0.clear();
    street_segs_zoom_1.clear();
    street_segs_zoom_2.clear();
}


// load extra intersection info into MAP
// Also determines the max and min LatLon for drawing
void load_intersections () {
    MAP.intersection_db.resize(getNumIntersections());
    MAP.intersection_node.resize(getNumIntersections());
    
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
        MAP.intersection_node[i] = (new Node(i, NO_EDGE, 0));
        
        //Check and update min/max lat/lon in world_values
        MAP.world_values.max_lat = std::max(MAP.world_values.max_lat, MAP.intersection_db[i].position.lat());
        MAP.world_values.min_lat = std::min(MAP.world_values.min_lat, MAP.intersection_db[i].position.lat());
        MAP.world_values.max_lon = std::max(MAP.world_values.max_lon, MAP.intersection_db[i].position.lon());
        MAP.world_values.min_lon = std::min(MAP.world_values.min_lon, MAP.intersection_db[i].position.lon());
    }
    
    //initialize out of bounds
    MAP.state.last_selected_intersection = MAP.intersection_db.size() + 1;
    MAP.route_data.start_intersection = MAP.intersection_db.size() + 1;
    MAP.route_data.end_intersection = MAP.intersection_db.size() + 1;
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


void load_points_of_interest () {
    std::vector<std::pair<std::pair<double, double>, unsigned int>> poi_zoom_0;
    
    for (unsigned int i = 0; i < unsigned(getNumPointsOfInterest()); i++) {
        
        double x = x_from_lon(getPointOfInterestPosition(i).lon());
        double y = y_from_lat(getPointOfInterestPosition(i).lat());
        
        std::pair<std::pair<double, double>, unsigned int> point = std::make_pair(std::make_pair(x, y), i);
        
        poi_zoom_0.push_back(point);
    }
    
    MAP.poi_k2tree.root = MAP.poi_k2tree.make_tree(poi_zoom_0.begin(), poi_zoom_0.end(), 0, poi_zoom_0.size(), -1);
    
    poi_zoom_0.clear();
}


void load_features () {
    std::vector<std::pair<std::pair<double, double>, unsigned int>> feature_zoom_m1;
    std::vector<std::pair<std::pair<double, double>, unsigned int>> feature_zoom_2;
    
    for (unsigned int i = 0; i < unsigned(getNumFeatures()); i++) {
        // Loops through feature points, counting how many intersect with limits of map, if it's
        // 4 or more, the map may be surrounded by an ocean or other feature, so must always be drawn
        int intersect_count = 0;
        for (int j = 0; j < getFeaturePointCount(i); j++) {
            double x = x_from_lon(getFeaturePoint(j, i).lon());
            double y = y_from_lat(getFeaturePoint(j, i).lat());
            
            if (x <= x_from_lon(MAP.world_values.min_lon) || x >= x_from_lon(MAP.world_values.max_lon)
                || y <= y_from_lat(MAP.world_values.min_lat) || y >= y_from_lat(MAP.world_values.max_lat)) {
                intersect_count++;
            }

            std::pair<std::pair<double, double>, unsigned int> point = std::make_pair(std::make_pair(x, y), i);
            
            // Differentiate zoom level for features
            FeatureType feature_type = getFeatureType(i);
            switch(feature_type) {
                case Beach: feature_zoom_2.push_back(point); break;
                case Building: feature_zoom_2.push_back(point); break;
                case Stream: feature_zoom_2.push_back(point); break;
                default: feature_zoom_m1.push_back(point); break;
            }
        }
        if(intersect_count >= 4) MAP.permanent_features.push_back(i);
    }
    
    MAP.feature_k2tree.root = MAP.feature_k2tree.make_tree(feature_zoom_m1.begin(), feature_zoom_m1.end(), 0, feature_zoom_m1.size(), -1);
    MAP.feature_k2tree.insert_bulk(feature_zoom_2.begin(), feature_zoom_2.end(), MAP.feature_k2tree.root, 0, feature_zoom_2.size(), 2);
    
    feature_zoom_2.clear();
    feature_zoom_m1.clear();
}


int get_street_segment_importance(unsigned street_db_id) {
    const OSMWay *street = MAP.OSM_data.way_by_OSMID[getInfoStreetSegment(street_db_id).wayOSMID];
    
    for(unsigned i=0; i < getTagCount(street); i++) {
        std::string key, value;
        std::tie(key, value) = getTagPair(street, i);

        if(key == "highway") {
            if(value == "motorway")          
                return -1;
            else if(value == "trunk")
                return 0;
            else if(value == "primary")
                return 1;
            else if(value == "secondary")
                return 2;
            else if(value == "tertiary" || value == "secondary_link"  || value == "trunk_link" || value == "motorway_link"  || value == "primary_link")
                return 3;
            else 
                return 4;
        
        }
    } 
    
    return 4;
}

void clear_intersection_node() {
    
    for(int i = 0; i < getNumIntersections(); i++) {
        MAP.intersection_node[i]->best_time = 0;
        MAP.intersection_node[i]->edge_in = NO_EDGE;
    }        
}

void clear_map_data() {
    MAP.intersection_db.clear();
    
    MAP.street_db.clear();
    
    MAP.street_name_id_map.clear();
    
    MAP.permanent_features.clear();
    
    MAP.LocalStreetSegments.clear();
    
    MAP.state.last_selected_intersection = 10000000;
    
    //clear KD trees and set root pointers to null
    if(MAP.street_seg_k2tree.root != nullptr) delete MAP.street_seg_k2tree.root;
        MAP.street_seg_k2tree.root = nullptr;
    if(MAP.poi_k2tree.root != nullptr) delete MAP.poi_k2tree.root;
        MAP.poi_k2tree.root = nullptr;
    if(MAP.feature_k2tree.root != nullptr) delete MAP.feature_k2tree.root;
        MAP.feature_k2tree.root = nullptr;
    
    // Clear every node intersection
    for(int i = 0; i < getNumIntersections(); i++) {
        delete MAP.intersection_node[i];
    }
        MAP.intersection_node.clear();
        
    MAP.OSM_data.bike_parking.clear();
    MAP.OSM_data.bike_routes.clear();
    MAP.OSM_data.node_by_OSMID.clear();
    MAP.OSM_data.subway_routes.clear();
    MAP.OSM_data.way_by_OSMID.clear();
    
    MAP.route_data.route_segments.clear();
    
    MAP.directions_data.clear();
}
