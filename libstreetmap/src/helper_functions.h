/*
 * Helper functions, mostly used for converting coordinates as well as 
 * some others
 */

#pragma once //protects against multiple inclusions of this header file

#include <vector>
#include <ezgl/point.hpp>
#include "ezgl/graphics.hpp"
#include <stdint.h>

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

//returns angle from 2 points
double angle_from_2_point2d (ezgl::point2d p1, ezgl::point2d p2);

//convert LatLon to point2d
ezgl::point2d point2d_from_LatLon (LatLon point);

//Pre-compute length of street 
double street_segment_length_helper(unsigned street_segment_id);

// calculate length between two points
double distance_from_points(double x1, double y1, double x2, double y2);

//returns the center point to draw the png with
ezgl::point2d png_draw_center_point(ezgl::renderer &g, ezgl::point2d original, int png_size);

//returns the bottom (y) and middle (x) point to draw the png with
ezgl::point2d png_draw_bottom_middle(ezgl::renderer &g, ezgl::point2d original, int png_height, int png_width);

//converts time in seconds to approximate time
std::string get_approximate_time(double time);

//converts time in seconds to readable time
std::string get_readable_time(int time);

//converts distance in meters to readable distance
std::string get_readable_distance(int distance);

//finds the common intersection between two connected street segments
unsigned find_common_intersection(unsigned street_segment1_id, unsigned street_segment2_id);

// Checks if one point is within a tolerance of another
bool check_intersection(double x1, double y1, double tolerance, double x2, double y2);

float fast_exp(float x) __attribute__ ((hot));