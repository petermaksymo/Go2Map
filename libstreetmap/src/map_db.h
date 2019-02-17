/*
 * Contains the definition for the global MAP object as well as definitions
 * for helper functions used to populate the MAP object that are called in 
 * load_map
 */

#pragma once

#include "StreetsDatabaseAPI.h"
#include <map>

//The definition of the global MAP object
//used as the main database

struct InfoIntersections {
    std::vector<unsigned> connected_street_segments;
    LatLon position;
    std::string name;
};

struct InfoStreets {
    std::vector<unsigned> segments;
    std::vector<unsigned> intersections;
};

struct InfoStreetSegmentsLocal {
    std::vector<double> street_segment_length;
    std::vector<double> street_segment_speed_limit; 
};

// The main structure for the globally defined MAP
struct MapInfo {
    std::vector<InfoIntersections> intersection_db;     //all intersections
    std::vector<InfoStreets> street_db;                 //all streets
    std::multimap<std::string, int> street_name_id_map; //for street names
    InfoStreetSegmentsLocal LocalStreetSegments;        //distances/speed limits for street segments
};


//tells compiler that MAP exists (so we can use it in all files)
//MAP is declared in map_object.cpp
extern MapInfo MAP;


//helper functions used in load_map
void load_street_segments ();

void load_intersections ();

void load_streets ();