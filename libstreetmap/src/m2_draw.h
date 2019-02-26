/*
 * This file contains functions to draw objects to the main canvas
 */

#include "ezgl/graphics.hpp"

#pragma once 

// Draws the last highlighted intersection stored in MAP
void draw_selected_intersection (ezgl::renderer &g);

// Queries street_seg_k2tree for the given zoom level to get the ids of all the
// segments that are current view. Then loops over the ids, drawing each curves
void draw_street_segments (ezgl::renderer &g);

// Uses a range query based on the current view and zoom level to find all the 
// street segments for this view and draw them
void draw_street_name(ezgl::renderer &g);

// Uses a range query based on the current view and zoom level to find
// the POIs only up to a calculated search_depth, which declusters the POIs
// by only going a certain depth into the k2tree
// Also draws street names
void draw_points_of_interest (ezgl::renderer &g);

// Uses a range query to find the features that have points in the current view,
// if no features are found, sets range to max bounds (for inside very large features),
// and also adds all permanent features, ie features that intersect the outerbounds.
// The permanent features account for large features such as an ocean surrounding an island.
void draw_features (ezgl::renderer &g);

// Draws subway data for all of the routes and  subway stops. Subways stops
// are only drawn above zoom level 2. This does not use KD Trees as subway routes
// are generally smaller data sets and stops are only drawn above zoom level 2.
void draw_subway_data(ezgl::renderer &g);

// Draws bike data including all bike paths and some of the bike parking, depending
// on the zoom_level. Does not use KD2Tree since this is generally a smaller
// dataset.
void draw_bike_data(ezgl::renderer &g);

//helper function to draw curves from LatLon point vector
void draw_curve(ezgl::renderer &g, std::vector<LatLon> &points);

//helper function to draw curves from a point2d vector
void draw_curve(ezgl::renderer &g, std::vector<ezgl::point2d> &points);

