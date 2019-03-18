/*
 * callback functions used in application.run of m2.cpp
 */

#include "map_db.h"
#include "helper_functions.h"
#include "m1.h"
#include "m2_callbacks.h"
#include <m3.h>
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include "constants.hpp"
#include <boost/algorithm/string.hpp>

#define LEFT_MOUSE_BUTTON 1
#define RIGHT_MOUSE_BUTTON 3

void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y) {
    
    //highlight intersection when clicked on
    if(event->button == LEFT_MOUSE_BUTTON) {
        //This part was used from the slides in tutorial
        //clear previously selected intersection
        if(MAP.state.last_selected_intersection <= getNumIntersections()) {
            MAP.intersection_db[MAP.state.last_selected_intersection].is_selected = false;
        }

        LatLon position = LatLon(lat_from_y(y), lon_from_x(x));
        int id = find_closest_intersection(position);
        // Get intersections and street segments
        std::cout << MAP.intersection_db[id].name << ": " << id << std::endl;
        std::vector<unsigned int> intersects = find_intersection_street_segments(id);
        for(std::vector<unsigned int>::iterator it = intersects.begin(); it != intersects.end(); it++) {
            std::cout << "int_id: " << *it << std::endl;
        }

        app->update_message("Closest Intersection: " + MAP.intersection_db[id].name);
        MAP.intersection_db[id].is_selected = true;
        MAP.state.last_selected_intersection = id;

        app->refresh_drawing();
        //end of use from tutorial slides
    } else if (event->button == RIGHT_MOUSE_BUTTON) {
        //find closest intersection and update state with it
        LatLon position = LatLon(lat_from_y(y), lon_from_x(x));
        MAP.state.directions_intersection_id = find_closest_intersection(position);;
        
        //give menu popup on right click
        GtkMenu *popup = (GtkMenu *)app->get_object("RightClickPopUp");
        gtk_menu_popup_at_pointer (popup, (GdkEvent*)event);
    }
}


// Shortcut keys for easy navigation and handles search
void act_on_key_press(ezgl::application *app, GdkEventKey *event, char *key_name) {
    (void) key_name;
    
    if(event->type == GDK_KEY_PRESS) {
        std::string main_canvas_id = app->get_main_canvas_id();
        auto canvas = app->get_canvas(main_canvas_id);
        
        //check if shortcut key
        switch(event->keyval) {
            case GDK_KEY_Page_Up:   ezgl::zoom_in(canvas, 5.0/3.0);      break;
            case GDK_KEY_Page_Down: ezgl::zoom_out(canvas, 5.0/3.0);     break;
            case GDK_KEY_Home:      
                ezgl::zoom_fit(canvas, canvas->get_camera().get_initial_world());
                break;
            case GDK_KEY_Escape:    app->quit();                         break;
            default: break;
        }
        
        // Predict searching as user types
        if (MAP.state.search_changed && event->keyval == GDK_KEY_Return) {
            GtkEntry* text_entry;
            
            // Pick which search bar to get text from depending on which has focus
            if(gtk_widget_is_focus((GtkWidget *) app->get_object("SearchBar"))) {
                text_entry = (GtkEntry *) app->get_object("SearchBar");
                MAP.state.displaying_search_results = true;
            }
            else if(gtk_widget_is_focus((GtkWidget *) app->get_object("ToBar"))) {
                text_entry = (GtkEntry *) app->get_object("ToBar");
                MAP.state.displaying_search_results = false;
            }
            else return;

            std::string text = gtk_entry_get_text(text_entry);
            boost::algorithm::to_lower(text);
                        
            std::string entry;
            if (text.find('&') == std::string::npos) {
                entry = text;
            } else {
                entry = text.substr(0, text.find('&') - 1);
            }
                        
            std::vector<unsigned> result;
            if (!entry.empty()) {
                if(check_and_switch_map(app, text)) {
                   return; 
                } else {
                    result = find_street_ids_from_partial_street_name(entry);
                }
            }
            
            int num_result_shown = MAX_SUGGESTIONS;
            // Limit search results shown
            if (result.size() < MAX_SUGGESTIONS) num_result_shown = result.size();
            
            if (num_result_shown != 0) {                 
                //loads the popup menu
                GtkMenu *popup = (GtkMenu *)app->get_object("SearchPopUp");
                GtkWidget *to_bar = (GtkWidget *)app->get_object("ToBar");

                //creates the popup menu under the search bar
                gtk_menu_popup_at_widget(popup, to_bar, GDK_GRAVITY_SOUTH, GDK_GRAVITY_NORTH,  NULL);
                
                //populate the menu with suggestions
                for (int i = 0; i < MAX_SUGGESTIONS; i++) {
                    std::string menu_item_id = "suggestion";
                    menu_item_id += std::to_string(i);

                    GtkWidget *suggestion = (GtkWidget *)app->get_object(menu_item_id.c_str());
                    
                    if(i >= (int)result.size()) {
                        //if no result, populate menu with a blank
                        gtk_menu_item_set_label((GtkMenuItem *)suggestion, "");
                    } else {
                        gtk_menu_item_set_label((GtkMenuItem *)suggestion, getStreetName(result[i]).c_str());
                    }
                }

                gtk_entry_grab_focus_without_selecting(text_entry);
            }
        }
    }
}


// Search for common intersections among all possible street results
std::vector<unsigned> search_intersection(std::string street1, std::string street2) {
    std::vector<unsigned> streetID1 = find_street_ids_from_partial_street_name(street1);
    std::vector<unsigned> streetID2 = find_street_ids_from_partial_street_name(street2);
    std::vector<unsigned> intersectionID, current_intersection;
    
    // Check for all possible intersections
    for (unsigned int i = 0; i < streetID1.size(); i++) {
        for (unsigned int j = 0; j < streetID2.size(); j++) {
            current_intersection = find_intersection_ids_from_street_ids(streetID1[i], streetID2[j]);
            intersectionID.insert(intersectionID.end(), current_intersection.begin(), current_intersection.end()); // Combine two vectors
        }
    }

    return intersectionID;
}

std::vector<unsigned> find_intersections_from_text(std::string &text, std::string &street1, std::string &street2, ezgl::application *ezgl_app) {
    // Parse the search result
    if (text.find('&') != std::string::npos) {
        std::stringstream ss(text.substr(text.find('&')));
        std::string second_street;
        ss >> second_street;
        ss >> second_street;
        if (text.find('&') == text.size() - 1 || text.find('&') == text.size() - 2) {
            ezgl_app->update_message("Second street needed");
            return {};
        //if (second_street == NULL) ezgl_app->update_message("Second street needed");
        } else {
            street1 = text.substr(0, text.find('&') - 1);
            street2 = text.substr(text.find('&') + 2);
            return search_intersection(street1, street2);
        }

    }
 }

void act_on_find(GtkWidget *widget, gpointer data) {
    (void) widget;
    
    // Pre declaration of the parameter of application
    auto ezgl_app = static_cast<ezgl::application *>(data);
    std::string main_canvas_id = ezgl_app->get_main_canvas_id();
    auto canvas = ezgl_app->get_canvas(main_canvas_id);
    
    // Extract text from the search bar
    GtkEntry* text_entry = (GtkEntry *) ezgl_app->get_object("SearchBar");
    std::string text = gtk_entry_get_text(text_entry);
    std::string street1, street2;
    if (MAP.state.search_word != text) {
        MAP.state.search_word = text;
        MAP.state.search_index = 0;
        
        // Parse the search result
        MAP.state.intersection_search_result = find_intersections_from_text(text, street1, street2, ezgl_app);
    } else MAP.state.search_index += 1;
       
    if (MAP.state.search_index == (int)MAP.state.intersection_search_result.size()) MAP.state.search_index = 0;
    int index = MAP.state.search_index;
    
    // Get location of the intersection and then apply margin to it
    if (MAP.state.intersection_search_result.empty()) {
        ezgl_app->update_message("No intersection found");
        
    } 
    else {
        // Translate to center intersection on screen
        LatLon pos = getIntersectionPosition(MAP.state.intersection_search_result[index]);
        ezgl::translate(canvas,
                        x_from_lon(pos.lon()) - (MAP.state.current_view_x.first + (MAP.state.current_view_x.second - MAP.state.current_view_x.first)/2),
                        y_from_lat(pos.lat()) - (MAP.state.current_view_y.first + (MAP.state.current_view_y.second - MAP.state.current_view_y.first)/2));
        // Zoom to 1km
        double zoom_scale = MAP.state.current_width / 1000; // Zoom into a width of 1km
        ezgl::zoom_in(canvas, zoom_scale);
        
        // Update global intersection and start global
        MAP.state.last_selected_intersection = MAP.state.intersection_search_result[index];
        
        // Update detail information of intersection and refresh the canvas
        ezgl_app->update_message(MAP.intersection_db[MAP.state.intersection_search_result[index]].name);
        ezgl_app->refresh_drawing();
    }
}


void act_on_transit_toggle(ezgl::application *app, bool isToggled) {
    MAP.state.is_transit_on = isToggled;
    
    app->refresh_drawing();
}  
    

void act_on_bikes_toggle(ezgl::application *app, bool isToggled) {
    MAP.state.is_bikes_on = isToggled;
  
    app->refresh_drawing();
}


void act_on_poi_toggle(ezgl::application *app, bool isToggled) {
    MAP.state.is_poi_on = isToggled;
  
    app->refresh_drawing();
}


void act_on_suggested_clicked(ezgl::application *app, std::string suggestion) {
    GtkEntry* text_entry;
    
    // Set text_entry based on which TextEntry the results correspond to
    if(MAP.state.displaying_search_results) text_entry = (GtkEntry *) app->get_object("SearchBar");
    else text_entry = (GtkEntry *) app->get_object("ToBar");
    
    std::string text = gtk_entry_get_text(text_entry);
    // Place in different formats depending whether it is the first or second street-
    if (text.find('&') == std::string::npos) suggestion = " & " + suggestion;
    else {
        suggestion = suggestion + " " + text.substr(text.find('&'));
    }
    gtk_entry_set_text(text_entry, suggestion.c_str());
}

//Generate directions
void act_on_directions(GtkWidget *widget, gpointer data) {
    auto ezgl_app = static_cast<ezgl::application *>(data);
    
    // Check SearchBar for valid intersections
    GtkEntry* text_entry = (GtkEntry *) ezgl_app->get_object("SearchBar");
    std::string text = gtk_entry_get_text(text_entry);
    std::string street1, street2;
    std::vector<unsigned> intersections_from = find_intersections_from_text(text, street1, street2, ezgl_app);
    if(intersections_from.size() == 0) {
        ezgl_app->update_message("Unable to find the \"from\" intersection. Please follow format \"street 1 & street 2\" ");
        ezgl_app->refresh_drawing();
        return;
    }
    
    // Check ToBar for valid intersections
    text_entry = (GtkEntry *) ezgl_app->get_object("ToBar");
    text = gtk_entry_get_text(text_entry);
    std::vector<unsigned> intersections_to = find_intersections_from_text(text, street1, street2, ezgl_app);
    if(intersections_to.size() == 0) {
        ezgl_app->update_message("Unable to find intersection \"to\" intersection. Please follow format \"street 1 & street 2\" ");
        ezgl_app->refresh_drawing();
        return;
    }
    
    // intersection vectors now have size > 1
    MAP.route_data.start_intersection = intersections_from[0];
    MAP.route_data.end_intersection = intersections_to[0];
    
    MAP.route_data.route_segments.clear();
    std::vector<unsigned> results = find_path_between_intersections(MAP.route_data.start_intersection,
                                                                    MAP.route_data.end_intersection, 0, 0);
    MAP.route_data.route_segments.insert(MAP.route_data.route_segments.end(), results.begin(), results.end());
    
    ezgl_app->refresh_drawing();
    
    results.clear();
}

bool check_and_switch_map(ezgl::application *app, std::string choice) {
    auto map_choice = valid_map_paths.find(choice);
    if (map_choice != valid_map_paths.end()) {
        std::cout << "Loading " + map_choice->first + " (it may take several seconds)\n";

        //should work but normally doesnt
        app->update_message("Loading " + map_choice->first + " (it may take several seconds)");
        app->refresh_drawing();

        close_map();

        load_map(map_choice->second);

        std::string main_canvas_id = app->get_main_canvas_id();
        auto canvas = app->get_canvas(main_canvas_id);

        ezgl::rectangle initial_world(
            {x_from_lon(MAP.world_values.min_lon), y_from_lat(MAP.world_values.min_lat)}, 
            {x_from_lon(MAP.world_values.max_lon), y_from_lat(MAP.world_values.max_lat)}
        );

        //reset the initial_world and the world
        canvas->get_camera().set_initial_world(initial_world);
        canvas->get_camera().set_world(initial_world);

        app->update_message("Successfully loaded  " + map_choice->first);
        app->refresh_drawing();
        
        return true;
    }
    
    return false;
}