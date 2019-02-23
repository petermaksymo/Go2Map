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

//Loads all needed OSM data as well as generates hash tables for nodes and ways
//so we can lookup through OSMid
void load_osm_data();

//Adds a subway route given the proper OSMRelation*
void add_subway_route(const OSMRelation* relation);

//Adds more ttc stations specifically because the OSM data is old
void add_ttc_station(const OSMNode* node, bool &found_ttc);

