/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m2.h
 * Author: liuhanke
 *
 * Created on February 25, 2019, 9:37 PM
 */

#ifndef M2_H
#define M2_H

void draw_map();

//
void draw_main_canvas (ezgl::renderer &g);

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

//
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y);

//
void act_on_key_press(ezgl::application *app, GdkEventKey *event, char *key_name);

//
void act_on_transit_toggle(ezgl::application *app, bool isToggled);

//
void act_on_bikes_toggle(ezgl::application *app, bool isToggled);

// 
void act_on_suggested_clicked(ezgl::application *app, std::string suggestion);

// 
bool check_and_switch_map(ezgl::application *app, std::string choice);

// Search all possible intersections between two roads return a vector to MAP data structure
void search_intersection(std::string street1, std::string street2);

#endif /* M2_H */

