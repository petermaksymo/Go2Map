/*
 * Contains the definition for the global MAP object as well as definitions
 * for helper functions used to populate the MAP object that are called in 
 * load_map
 */

#pragma once

#include "StreetsDatabaseAPI.h"
#include <map>
#include "KD2Tree.h"
#include <unordered_map>
#include <ezgl/point.hpp>

//The definition of the global MAP object
//used as the main database

struct InfoIntersections {
    std::vector<unsigned> connected_street_segments;
    LatLon position;
    std::string name;
    bool is_selected = false;
};

struct InfoStreets {
    std::vector<unsigned> segments;
    std::vector<unsigned> intersections;
};

struct InfoStreetSegmentsLocal {
    std::vector<double> street_segment_length;
    std::vector<double> street_segment_speed_limit; 
};

struct WorldValues {
    double max_lat; //maximum latitude for drawing
    double max_lon; //maximum longitude for drawing
    double min_lat; //minimum latitude for drawing
    double min_lon; //minimum longitude for drawing
};

//Values that change throughout map navigation
struct Map_State {
    int last_selected_intersection;  //last selected intersection to wipe selected
    double scale; // Scale showing how zoomed the map is
    std::vector<unsigned> intersection_search_result;
    std::pair<double, double> current_view_y;
    std::pair<double, double> current_view_x;
    int zoom_level;
};

struct SubwayRouteData {
    std::string colour;
    std::vector<ezgl::point2d> stations;
    std::vector<std::vector<ezgl::point2d>> path;
};

struct OSMData {
    std::unordered_map<OSMID, const OSMNode*> node_by_OSMID; //hash table for search by OSMID
    std::unordered_map<OSMID, const OSMWay*> way_by_OSMID;   //hash table for search by OSMID
    std::vector<SubwayRouteData> subway_routes;
};

// The main structure for the globally defined MAP
struct MapInfo {
    std::vector<InfoIntersections> intersection_db;     //all intersections
    std::vector<InfoStreets> street_db;                 //all streets
    std::multimap<std::string, int> street_name_id_map; //for street names
    InfoStreetSegmentsLocal LocalStreetSegments;        //distances/speed limits for street segments
    WorldValues world_values;                           //values about the world (e.g. max latitude))
    Map_State state;
    KD2Tree street_seg_k2tree;
    OSMData OSM_data;
};


//tells compiler that MAP exists (so we can use it in all files)
//MAP is declared in map_object.cpp
extern MapInfo MAP;


//helper functions used in load_map
void load_street_segments ();

void load_intersections ();

void load_streets ();
