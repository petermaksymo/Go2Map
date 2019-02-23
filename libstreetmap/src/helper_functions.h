/*
 * File:   helper_functions.h
 *
 */

#pragma once //protects against multiple inclusions of this header file

#include <vector>
#include <ezgl/point.hpp>

//sorts and removes all duplicates in a vector
void removeDuplicates(std::vector<unsigned>& vec);

//convert latitude to y coordinate
double y_from_lat (double lat); 
    
//convert longitude to x coordinate
double x_from_lon (double lon);

//convert y coordinate to latitude
double lat_from_y (double y);

//convert x coordinate to longitude
double lon_from_x (double x);

//convert LatLon to point2d
ezgl::point2d point2d_from_LatLon (LatLon point);

//Pre-compute length of street 
double street_segment_length_helper(unsigned street_segment_id);

