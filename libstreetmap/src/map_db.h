/*
 * Contains the definition for the global MAP object as well as definitions
 * for helper functions used to populate the MAP object that are called in 
 * load_map
 */

#pragma once

#include "StreetsDatabaseAPI.h"
#include <map>
#include <list>
#include "KD2Tree.h"
#include <unordered_map>
#include <ezgl/point.hpp>
#include "m3.h"
#include "ezgl/application.hpp"

//The definition of the global MAP object
//used as the main database
// A node represent a single intersection of the map
class Node {
    public:
        
    int intersection_id; // ID of intersection the node contains
    int edge_in; // The route taken coming into the node
    double best_time;
    std::vector<int> edge_out; // All street segment going out of the intersection  
    
    Node(int id, int edge, double time) {
        intersection_id = id;
        edge_in = edge;
        best_time = time;
        
        // Insert every single connected street segment into outgoing_
        for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id); i++) {
            edge_out.push_back(getIntersectionStreetSegment(i, intersection_id));
        }
        
    }
    
};

// A wave element to traverse through all the nodes
class waveElem {
    public:
    
    Node* node;
    int edgeID;
    double travel_time;
    
    waveElem(Node* node1, int edgeID1, double travel_time1) {
        node = node1;
        edgeID = edgeID1;
        travel_time = travel_time1;
    }
};

// A custom comparator used to sort min heap base on travel_time
class comparator { 
    public: 
    int operator() (const waveElem point1, const waveElem point2) 
    { 
        return point1.travel_time > point2.travel_time; 
    } 
}; 

struct InfoIntersections {
    std::vector<unsigned> connected_street_segments;
    LatLon position;
    std::string name;
    bool is_selected = false;
};

struct InfoStreets {
    std::vector<unsigned> segments;
    std::vector<unsigned> intersections;
    double length;
    double average_speed;
};

struct InfoStreetSegmentsLocal {
    double street_segment_length;
    double street_segment_speed_limit; 
    double travel_time;
    int importance_level;               //when to draw based off zoom-level (-1 is most important)
};

struct WorldValues {
    double max_lat; //maximum latitude for drawing
    double max_lon; //maximum longitude for drawing
    double min_lat; //minimum latitude for drawing
    double min_lon; //minimum longitude for drawing
};

struct RouteData {
    std::vector<unsigned int> route_segments; // vector of street segments in route
    unsigned int start_intersection;          // id of start of route
    unsigned int end_intersection;            // id of end of route
};

//Data string and turn type used for generating the directions
struct DirectionsData {
    std::string street;
    TurnType turn_type;
    std::string path_time;
    std::string path_distance;
    double lat;
    double lon;
};

//Values that change throughout map navigation
struct Map_State {
    int last_selected_intersection;  //last selected intersection to wipe selected
    double scale; // Scale showing how zoomed the map is
    std::vector<unsigned> intersection_search_result;
    std::pair<double, double> current_view_y;
    std::pair<double, double> current_view_x;
    std::pair<double, double> current_view_y_buffered;
    std::pair<double, double> current_view_x_buffered;
    int zoom_level; // 0: ('scale' < 4) most zoomed out, 1: ('scale' > 4 & 'scale' < 10) middle, 2: ('scale > 10) most zoomed in
    std::string search_word;
    int directions_intersection_id; //populated on right click for setting to/from
    int search_index = 0;
    bool is_transit_on = false;
    bool is_bikes_on = false;
    bool is_poi_on = true;
    bool search_changed = false;
    bool displaying_search_results = true; // true if results in dropdown correspond to "SearchBar", false if for "ToBar"
    double current_width;
    bool is_from_set_right_click = false;
    bool is_to_set_right_click = false;
    std::list<unsigned> visited_node;
};

struct SubwayRouteData {
    std::vector<ezgl::point2d> stations;
    std::vector<std::vector<ezgl::point2d>> path;
};


struct OSMData {
    std::unordered_map<OSMID, const OSMNode*> node_by_OSMID; //hash table for search by OSMID
    std::unordered_map<OSMID, const OSMWay*> way_by_OSMID;   //hash table for search by OSMID
    std::vector<SubwayRouteData> subway_routes;
    std::vector<std::vector<ezgl::point2d>> bike_routes;
    std::vector<ezgl::point2d> bike_parking;
};

struct Courier {
    std::vector<std::vector<unsigned>> time_between_deliveries; // A two dimentional array for time between delivery locations
}; 

// The main structure for the globally defined MAP
struct MapInfo {
    std::vector<InfoIntersections> intersection_db;     //all intersections
    std::vector<Node*> intersection_node;  // all the intersection as node
    std::vector<InfoStreets> street_db;   
    std::vector<unsigned int> permanent_features; // features that must always be drawn
    std::multimap<std::string, int> street_name_id_map; //for street names
    std::vector<InfoStreetSegmentsLocal> LocalStreetSegments;        //distances/speed limits for street segments
    WorldValues world_values;                           //values about the world (e.g. max latitude))
    Map_State state;
    KD2Tree street_seg_k2tree;
    KD2Tree poi_k2tree;
    KD2Tree feature_k2tree;
    OSMData OSM_data;
    RouteData   route_data;
    Courier courier;
    std::vector<DirectionsData> directions_data;
    std::string travel_time;
    std::string travel_distance;
    int highlighted_direction = -1;
    double screen_width;
    double screen_height;
};

//tells compiler that MAP exists (so we can use it in all files)
//MAP is declared in map_object.cpp
extern MapInfo MAP;


//helper functions used in load_map
void load_street_segments ();

void load_intersections ();

void load_streets ();

void load_points_of_interest ();

void load_features ();

int get_street_segment_importance(unsigned street_db_id);

// Clear time and edge information after a route search
void clear_intersection_node();

//clears all data for reloading maps
void clear_map_data();
