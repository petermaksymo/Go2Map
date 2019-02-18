/*
 * File:   helper_functions.h
 *
 */

#pragma once //protects against multiple inclusions of this header file

#include <vector>

//sorts and removes all duplicates in a vector
void removeDuplicates(std::vector<unsigned>& vec);

//convert latitude to y coordinate
double lat_to_y (double lat); 
    
//convert longitude to x coordinate
double lon_to_x (double lon);

//Pre-compute length of street 
double street_segment_length_helper(unsigned street_segment_id);

