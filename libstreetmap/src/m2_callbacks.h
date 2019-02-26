/*
 * callback functions used in application.run of m2.cpp
 */

#include "ezgl/application.hpp"
#include <string>

#pragma once 


//
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y);

//
void act_on_key_press(ezgl::application *app, GdkEventKey *event, char *key_name);

//
void act_on_transit_toggle(ezgl::application *app, bool isToggled);

//
void act_on_bikes_toggle(ezgl::application *app, bool isToggled);

//
void act_on_poi_toggle(ezgl::application *app, bool isToggled);

// 
void act_on_suggested_clicked(ezgl::application *app, std::string suggestion);

// 
bool check_and_switch_map(ezgl::application *app, std::string choice);

// Search all possible intersections between two roads return a vector to MAP data structure
void search_intersection(std::string street1, std::string street2);


