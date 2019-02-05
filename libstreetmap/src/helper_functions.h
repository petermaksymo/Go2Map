/* 
 * File:   helper_functions.h
 *
 */

#pragma once //protects against multiple inclusions of this header file

#include <vector>

//sorts and removes all duplicates in a vector
void removeDuplicates(std::vector<unsigned>& vec);

//Pre-compute length of street 
double street_segment_length_helper(unsigned street_segment_id);

