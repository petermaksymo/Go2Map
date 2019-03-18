/*
 * callback functions used in application.run of m2.cpp
 */

#include "ezgl/application.hpp"
#include <string>

#pragma once 


//Used for highlighting an intersection when clicked on
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y);

//Used for shortcut keys and handling search
void act_on_key_press(ezgl::application *app, GdkEventKey *event, char *key_name);

//Changes MAP state to reflect the transit checkbox
void act_on_transit_toggle(ezgl::application *app, bool isToggled);

//Changes MAP state to reflect the bikes checkbox
void act_on_bikes_toggle(ezgl::application *app, bool isToggled);

//Changes MAP state to reflect the POI checkbox
void act_on_poi_toggle(ezgl::application *app, bool isToggled);

//Populate search field with selected suggestion
void act_on_suggested_clicked(ezgl::application *app, std::string suggestion);

// Will run search
void act_on_find(GtkWidget *widget, gpointer data);

//will run pathfinding
void act_on_directions(GtkWidget *widget, gpointer data);

//Helpers:
//used in searching to check if we should switch and do it if possible
bool check_and_switch_map(ezgl::application *app, std::string choice);

// Search all possible intersections between two roads return a vector of resulting intersections
std::vector<unsigned> search_intersection(std::string street1, std::string street2);

// Parses streets from text (with &) and returns corresponding street segments or prints the appropriate errors
std::vector<unsigned> find_intersections_from_text(std::string &text, std::string &street1, std::string &street2, ezgl::application *ezgl_app);


