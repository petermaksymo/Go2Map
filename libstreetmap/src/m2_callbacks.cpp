/*
 * callback functions used in application.run of m2.cpp
 */

#include "map_db.h"
#include "helper_functions.h"
#include "m1.h"
#include "m2_callbacks.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include "constants.hpp"
#include <boost/algorithm/string.hpp>

void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y) {
    
    //highlight intersection when clicked on
    if(event->button == 1) {
        //This part was used from the slides in tutorial
        //clear previously selected intersection
        if(MAP.state.last_selected_intersection <= getNumIntersections()) {
            MAP.intersection_db[MAP.state.last_selected_intersection].is_selected = false;
        }

        std::cout << "Mouse clicked at (" << x << ", " << y << ")\n";

        LatLon position = LatLon(lat_from_y(y), lon_from_x(x));
        int id = find_closest_intersection(position);

        app->update_message("Closest Intersection: " + MAP.intersection_db[id].name);
        MAP.intersection_db[id].is_selected = true;
        MAP.state.last_selected_intersection = id;
        std::cout << id << std::endl;

        app->refresh_drawing();
        //end of use from tutorial slides
    } 
}

// Shortcut keys for easy navigation
void act_on_key_press(ezgl::application *app, GdkEventKey *event, char *key_name) {
    (void) key_name;
    if(event->type == GDK_KEY_PRESS) {
        std::string main_canvas_id = app->get_main_canvas_id();
        auto canvas = app->get_canvas(main_canvas_id);
        
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
            GtkEntry* text_entry = (GtkEntry *) app->get_object("SearchBar");
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
                std::cout<< std::endl;
                
                //loads the popup menu
                GtkMenu *popup = (GtkMenu *)app->get_object("SearchPopUp");
                GtkWidget *search_bar = (GtkWidget *)app->get_object("SearchBar");

                //creates the popup menu under the search bar
                gtk_menu_popup_at_widget(popup, search_bar, GDK_GRAVITY_SOUTH, GDK_GRAVITY_NORTH,  NULL);
                
                //populate the menu with suggestions
                for (int i = 0; i < MAX_SUGGESTIONS; i++) {
                    std::string menu_item_id = "suggestion";
                    menu_item_id += std::to_string(i);

                    GtkWidget *suggestion = (GtkWidget *)app->get_object(menu_item_id.c_str());
                    
                    if(i >= (int)result.size()) {
                        //if no result, populate menu with a blank
                        gtk_menu_item_set_label((GtkMenuItem *)suggestion, "");
                    } else {
                        std::cout << getStreetName(result[i]) << std::endl;

                        gtk_menu_item_set_label((GtkMenuItem *)suggestion, getStreetName(result[i]).c_str());
                    }
                }

                gtk_entry_grab_focus_without_selecting(text_entry);
            }
        }
    }
}

// Search for common intersections among all possible street results
void search_intersection(std::string street1, std::string street2) {
    std::vector<unsigned> streetID1 = find_street_ids_from_partial_street_name(street1);
    std::vector<unsigned> streetID2 = find_street_ids_from_partial_street_name(street2);
    std::vector<unsigned> intersectionID, current_intersection;
    
    // Check for all possible intersections
    for (unsigned int i = 0; i < streetID1.size() - 1; i++) {
        for (unsigned int j = 0; j < streetID2.size() - 1; j++) {
            current_intersection = find_intersection_ids_from_street_ids(streetID1[i], streetID2[j]);
            intersectionID.insert(intersectionID.end(), current_intersection.begin(), current_intersection.end()); // Combine two vectors
        }
    }

    MAP.state.intersection_search_result = intersectionID;
    for (unsigned int i = 0; i < intersectionID.size(); i++) {
        std::cout << getIntersectionName(intersectionID[i]) << std::endl;
    }
}

gboolean ezgl::press_find(GtkWidget *widget, gpointer data) {
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
         if (text.find('&') != std::string::npos) {
             std::stringstream ss(text.substr(text.find('&')));
             std::string second_street;
             ss >> second_street;
             ss >> second_street;
             if (text.find('&') == text.size() - 1 || text.find('&') == text.size() - 2) ezgl_app->update_message("Second street needed");
             //if (second_street == NULL) ezgl_app->update_message("Second street needed");
             else {
                 street1 = text.substr(0, text.find('&') - 1);
                 street2 = text.substr(text.find('&') + 2);
                 search_intersection(street1, street2);
             }
             
        }
    } else MAP.state.search_index += 1;
    
    //Constant for reconstruct current view after zoom in after search
    const double margin = 0.0001;
    
   
    if (MAP.state.search_index == (int)MAP.state.intersection_search_result.size()) MAP.state.search_index = 0;
    int index = MAP.state.search_index;
    // Get location of the intersection and then apply margin to it
    if (MAP.state.intersection_search_result.empty()) {
        ezgl_app->update_message("No intersection found");
        
    } 
    else {
        LatLon pos = getIntersectionPosition(MAP.state.intersection_search_result[index]);
        ezgl::point2d origin(x_from_lon(pos.lon()) - margin, y_from_lat(pos.lat()) - margin);
        ezgl::point2d top_right(x_from_lon(pos.lon()) + margin, y_from_lat(pos.lat()) + margin);
        // Construct new view of the canvas
        rectangle view(origin, top_right);
        ezgl::zoom_fit(canvas, view);
        MAP.state.last_selected_intersection = MAP.state.intersection_search_result[index];
        // Update detail information of intersection and refresh the canvas
        ezgl_app->update_message(MAP.intersection_db[MAP.state.intersection_search_result[index]].name);
        ezgl_app->refresh_drawing();
    }
    return True;
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
    GtkEntry* text_entry = (GtkEntry *) app->get_object("SearchBar");
    std::string text = gtk_entry_get_text(text_entry);
    // Place in different formats depending whether it is the first or second street-
    if (text.find('&') == std::string::npos) suggestion = " & " + suggestion;
    else {
        suggestion = suggestion + " " + text.substr(text.find('&'));
    }
    gtk_entry_set_text(text_entry, suggestion.c_str());
}


bool check_and_switch_map(ezgl::application *app, std::string choice) {
    auto map_choice = valid_map_paths.find(choice);
    if (map_choice != valid_map_paths.end()) {
        std::cout << "Loading " + map_choice->first + " (it may take several seconds)\n";

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