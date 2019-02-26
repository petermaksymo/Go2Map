/*
 * This file contains functions to draw objects to the main canvas
 */

#include "ezgl/graphics.hpp"

#pragma once 

//
void draw_selected_intersection (ezgl::renderer &g);

//
void draw_street_segments (ezgl::renderer &g);

//
void draw_points_of_interest (ezgl::renderer &g);

//
void draw_features (ezgl::renderer &g);

//
void draw_street_name(ezgl::renderer &g);

//
void draw_subway_data(ezgl::renderer &g);

//
void draw_bike_data(ezgl::renderer &g);

//
void draw_curve(ezgl::renderer &g, std::vector<LatLon> &points);

//
void draw_curve(ezgl::renderer &g, std::vector<ezgl::point2d> &points);

