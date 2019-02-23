/*
 * The definitions to functions used to fetch extra data from the Layer-1 api
 */

/* 
 * File:   OSMFetchers.h
 * Author: maksymo4
 *
 * Created on February 22, 2019, 4:38 PM
 */

#pragma once

#include "OSMDatabaseAPI.h"
#include <vector>
#include "ezgl/point.hpp"

//Loads all needed OSM data as well as generates hash tables for nodes and ways
//so we can lookup through OSMid
void load_osm_data();

//goes through all OSM nodes creating a hash table for lookup by OSMID and
//adds any node data already distinguishable to MAP
void go_through_OSM_nodes();

//goes through all OSM ways creating a hash table for lookup by OSMID and
//adds any way data already distinguishable to MAP
void go_through_OSM_ways();

//goes through all OSM relations and builds the rest of MAP data
//note, must be run AFTER going through nodes/ways as it needs the hash tables
//for lookup by OSMID
void go_through_OSM_relations();

//Adds a subway route given the proper OSMRelation*
void add_subway_route(const OSMRelation* relation);

//Adds more ttc stations specifically because the OSM data is old
void add_ttc_station(const OSMNode* node, bool &found_ttc);

//creates a vector of point2d coordinates given a way
std::vector<ezgl::point2d> point_vector_from_way(const OSMWay* way);

