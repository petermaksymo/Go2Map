/*
 * Initial graphics implementation (may need to be refactored to separate files later)
 */

#include "map_db.h"
#include "helper_functions.h"
#include "m1.h"
#include "m2.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include "constants.hpp"
#include <boost/algorithm/string.hpp>

void draw_main_canvas (ezgl::renderer &g);
void draw_selected_intersection (ezgl::renderer &g);
void draw_street_segments (ezgl::renderer &g);
void draw_points_of_interest (ezgl::renderer &g);
void draw_features (ezgl::renderer &g);
void draw_street_name(ezgl::renderer &g);
void draw_subway_data(ezgl::renderer &g);
void draw_bike_data(ezgl::renderer &g);
void draw_curve(ezgl::renderer &g, std::vector<LatLon> &points);
void draw_curve(ezgl::renderer &g, std::vector<ezgl::point2d> &points);
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y);
void act_on_key_press(ezgl::application *app, GdkEventKey *event, char *key_name);
void act_on_transit_toggle(ezgl::application *app, bool isToggled);
void act_on_bikes_toggle(ezgl::application *app, bool isToggled);
void act_on_suggested_clicked(ezgl::application *app, std::string suggestion);
void show_search_result();
bool check_and_switch_map(ezgl::application *app, std::string choice);


void draw_map () {
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
    
    ezgl::rectangle initial_world(
            {x_from_lon(MAP.world_values.min_lon), y_from_lat(MAP.world_values.min_lat)}, 
            {x_from_lon(MAP.world_values.max_lon), y_from_lat(MAP.world_values.max_lat)}
    );
    
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);
    
    
    application.run(nullptr, act_on_mouse_click, 
                    nullptr, act_on_key_press,
                    act_on_transit_toggle, act_on_bikes_toggle,
                    act_on_suggested_clicked);
}


void draw_main_canvas (ezgl::renderer &g) {    
    // Calculate the scale every time main canvas is drawn
    ezgl::rectangle current_view = g.get_visible_world();
    MAP.state.current_view_x = std::make_pair(current_view.left(), current_view.right());
    MAP.state.current_view_y = std::make_pair(current_view.bottom(), current_view.top());
   
    MAP.state.scale = (x_from_lon(MAP.world_values.max_lon) - x_from_lon(MAP.world_values.min_lon)) / 
        (current_view.right() -current_view.left());
    
    // Set current width of view
    LatLon top_left(lon_from_x(current_view.left()), lat_from_y(current_view.top()));
    LatLon top_right(lon_from_x(current_view.right()), lat_from_y(current_view.top()));

    MAP.state.current_width = find_distance_between_two_points(top_left, top_right);

    // add a buffered current view to ensure the range prevent the zoom level
    // from creating too small a range and losing features
    double x_buffer;
    double y_buffer;
    if(MAP.state.current_width > 500) {
        // Used for map range queries to avoid corners being cut off
        x_buffer = (current_view.right() - current_view.left()) * 0.2;
        y_buffer = (current_view.top() - current_view.bottom()) * 0.2;
    } else {
        x_buffer = ((MAP.state.current_view_x_buffered.second - MAP.state.current_view_x_buffered.first) - (current_view.right() - current_view.left()))/2;
        y_buffer = ((MAP.state.current_view_y_buffered.second - MAP.state.current_view_y_buffered.first) - (current_view.top() - current_view.bottom()))/2;
    }
    
    MAP.state.current_view_x_buffered = std::make_pair(current_view.left() - x_buffer, current_view.right() + x_buffer);
    MAP.state.current_view_y_buffered = std::make_pair(current_view.bottom() - y_buffer, current_view.top() + y_buffer);
    
    if (MAP.state.current_width >= 25000)       MAP.state.zoom_level = 0;
    else if (MAP.state.current_width  >= 7500)  MAP.state.zoom_level = 1;
    else if (MAP.state.current_width >= 3000)   MAP.state.zoom_level = 2;
    else if (MAP.state.current_width >= 2000)   MAP.state.zoom_level = 3;
    else                                        MAP.state.zoom_level = 4;
        
    g.set_color(ezgl::BACKGROUND_GREY);
    
    g.fill_rectangle(
            {x_from_lon(MAP.world_values.min_lon), y_from_lat(MAP.world_values.min_lat)}, 
            {x_from_lon(MAP.world_values.max_lon), y_from_lat(MAP.world_values.max_lat)}
    );
    
    g.set_line_cap(ezgl::line_cap::round);
    
    draw_features(g);    
    draw_street_segments(g);
    if(MAP.state.is_bikes_on) draw_bike_data(g);
    draw_points_of_interest(g);
    draw_selected_intersection(g);
    draw_street_name(g);
    
    if(MAP.state.is_transit_on) draw_subway_data(g);
}


void draw_selected_intersection (ezgl::renderer &g) {
    int id = MAP.state.last_selected_intersection;
    if (id <= getNumIntersections()) {
        float x = x_from_lon(MAP.intersection_db[id].position.lon());
        float y = y_from_lat(MAP.intersection_db[id].position.lat());
        
        float radius = (x_from_lon(MAP.world_values.max_lon) - x_from_lon(MAP.world_values.min_lon))/5000;
        
        g.set_color(ezgl::GREEN);
        g.fill_arc(ezgl::point2d(x,y), radius, 0, 360);
    }
}

// Queries street_seg_k2tree for the given zoom level to get the ids of all the
// segments that are current view. Then loops over the ids, drawing each curves
void draw_street_segments (ezgl::renderer &g) {    
    
    std::map<unsigned int, std::pair<double, double>> result_ids;
    std::vector<std::pair<std::pair<double, double>, unsigned int>> result_points;
    
    MAP.street_seg_k2tree.range_query(MAP.street_seg_k2tree.root, // root
                         0, // depth of query
                         std::make_pair(MAP.state.current_view_x_buffered.first, MAP.state.current_view_x_buffered.second), // x-range (smaller, greater)
                         std::make_pair(MAP.state.current_view_y_buffered.first, MAP.state.current_view_y_buffered.second), // y-range (smaller, greater)
                         result_points, // results
                         result_ids,
                         MAP.state.zoom_level, 0); // zoom_level
    
    for(std::map<unsigned int, std::pair<double, double>>::iterator it = result_ids.begin(); it != result_ids.end(); it++) { 
        
        int id = it->first;
        // for (unsigned int id = 0; id < unsigned(getNumStreetSegments()); id++) {
        g.set_color(ezgl::WHITE);
        
        //load all LatLon of points into a vector for the draw_curve helper function
        std::vector<LatLon> points;
        points.push_back(MAP.intersection_db[getInfoStreetSegment(id).from].position);
        if(getInfoStreetSegment(id).curvePointCount > 0) {
            for(int i = 0; i < getInfoStreetSegment(id).curvePointCount; i++) {
                points.push_back(getStreetSegmentCurvePoint(i, id));
            }
        }
        points.push_back(MAP.intersection_db[getInfoStreetSegment(id).to].position);
        
        //set width before drawing
        if (MAP.state.zoom_level > 3) {
            g.set_line_width(MAP.LocalStreetSegments.street_segment_speed_limit[id] / 10);
        } else if (MAP.state.zoom_level == 0) {
            g.set_line_width(MAP.LocalStreetSegments.street_segment_speed_limit[id] / 30);
        } else {
            g.set_line_width(MAP.LocalStreetSegments.street_segment_speed_limit[id] / 20);
        }
        
        //make highways orange
        if(MAP.LocalStreetSegments.street_segment_speed_limit[id] >= 80) {
            g.set_color(ezgl::ORANGE);
        }
        
        draw_curve(g, points);
        
        // draw one way symbols on curve points
        if (getInfoStreetSegment(id).oneWay && MAP.state.zoom_level >= 4) {
            for(std::vector<LatLon>::iterator it_p = points.begin(); (it_p + 1) != points.end(); it_p++) {
                double x1 = x_from_lon(it_p->lon());
                double y1 = y_from_lat(it_p->lat());
                double x2 = x_from_lon((it_p + 1)->lon());
                double y2 = y_from_lat((it_p + 1)->lat());

                double angle;
                if(x2 == x1 && y2 > y1) angle = atan(1)*2 /DEG_TO_RAD; // pi / 2
                else if(x2 == x1 && y2 < y1) angle = atan(1)*6 /DEG_TO_RAD; // 3* pi / 2
                else angle = ( atan( (y2-y1)/(x2-x1) ) )/DEG_TO_RAD;

                //keep flip one way street symbol if should be pointing left
                if (x2 < x1) angle = angle - 180;
                
                g.set_color(ezgl::ORANGE);

                ezgl::point2d mid((x2 + x1) / 2.0, (y2 + y1) / 2.0);

                g.set_text_rotation(angle);
                g.draw_text(mid, ">", distance_from_points(x1, y1, x2, y2) / 2, 100);
            }
        }
    }
    
    result_ids.clear();
    result_points.clear();
}

void draw_street_name(ezgl::renderer &g) {
    
    std::map<unsigned int, std::pair<double, double>> result_ids;
    std::vector<std::pair<std::pair<double, double>, unsigned int>> result_points;
    
    MAP.street_seg_k2tree.range_query(MAP.street_seg_k2tree.root, // root
                         0, // depth of query
                         std::make_pair(MAP.state.current_view_x_buffered.first, MAP.state.current_view_x_buffered.second), // x-range (smaller, greater)
                         std::make_pair(MAP.state.current_view_y_buffered.first, MAP.state.current_view_y_buffered.second), // y-range (smaller, greater)
                         result_points, // results
                         result_ids,
                         MAP.state.zoom_level, 0); // zoom_level
    
    for(std::map<unsigned int, std::pair<double, double>>::iterator it = result_ids.begin(); it != result_ids.end(); it++) { 

        int i = it->first;
        
        // Set middle of text, accounting for potential curve points
        //load all LatLon of points into a vector for the draw_curve helper function
        std::vector<LatLon> points;
        points.push_back(MAP.intersection_db[getInfoStreetSegment(i).from].position);
        if(getInfoStreetSegment(i).curvePointCount > 0) {
            for(int j = 0; j < getInfoStreetSegment(i).curvePointCount; j++) {
                points.push_back(getStreetSegmentCurvePoint(j, i));
            }
        }
        points.push_back(MAP.intersection_db[getInfoStreetSegment(i).to].position);
        
        if (MAP.state.zoom_level >= 3 && (getStreetName(getInfoStreetSegment(i).streetID) != "<unknown>")) {
            for(std::vector<LatLon>::iterator it_p = points.begin(); (it_p + 1) != points.end(); it_p++) {
                double x1 = x_from_lon(it_p->lon());
                double y1 = y_from_lat(it_p->lat());
                double x2 = x_from_lon((it_p + 1)->lon());
                double y2 = y_from_lat((it_p + 1)->lat());

                double angle;
                if(x2 == x1 && y2 > y1) angle = atan(1)*2 /DEG_TO_RAD; // pi / 2
                else if(x2 == x1 && y2 < y1) angle = atan(1)*6 /DEG_TO_RAD; // 3* pi / 2
                else angle = ( atan( (y2-y1)/(x2-x1) ) )/DEG_TO_RAD;
                
                //keep orientation of text the same
                if (angle > 90) angle = angle - 180;
                else if (angle < -90) angle = angle + 180;
                
                g.set_color(ezgl::BLACK);
                
                ezgl::point2d mid((x2 + x1) / 2.0, (y2 + y1) / 2.0);
                
                
                g.set_text_rotation(angle);
                g.draw_text(mid, getStreetName(getInfoStreetSegment(i).streetID), distance_from_points(x1, y1, x2, y2), 100);
            }
        }
        points.clear();
    }
    
    result_ids.clear();
    result_points.clear();
}

void draw_points_of_interest (ezgl::renderer &g) {
    std::map<unsigned int, std::pair<double, double>> result_ids;
    std::vector<std::pair<std::pair<double, double>, unsigned int>> result_points;
    if(MAP.state.zoom_level >= 2) {
        MAP.poi_k2tree.range_query(MAP.poi_k2tree.root, // root
                             0, // depth of query
                             std::make_pair(MAP.state.current_view_x_buffered.first, MAP.state.current_view_x_buffered.second), // x-range (smaller, greater)
                             std::make_pair(MAP.state.current_view_y_buffered.first, MAP.state.current_view_y_buffered.second), // y-range (smaller, greater)
                             result_points, // results
                             result_ids,
                             MAP.state.zoom_level, 0); // zoom_level

        for(std::map<unsigned int, std::pair<double, double>>::iterator it = result_ids.begin(); it != result_ids.end(); it++) { 

        int i = it->first;
        
        double x = x_from_lon(getPointOfInterestPosition(i).lon());
        double y = y_from_lat(getPointOfInterestPosition(i).lat());
        std::string poi_name = getPointOfInterestName(i);
        
        float radius = (x_from_lon(MAP.world_values.max_lon) - x_from_lon(MAP.world_values.min_lon))/7500;
        
        // Reduce size of red dot as user zooms in to reduce clutter
        if (MAP.state.scale > 70) radius /= 2;
        if (MAP.state.scale > 130) radius /= 2;
        if (MAP.state.scale > 260) radius /= 2;
        
        // Draw the red dot representing POI
        g.set_color(ezgl::RED);   
        g.fill_arc(ezgl::point2d(x,y), radius, 0, 360);
        
        // Display text differently at different scale level
        if (MAP.state.scale > 130 && (i % 3  == 0)) {
            g.set_color(ezgl::SADDLE_BROWN);
            g.set_text_rotation(0);
            if (i % 2 == 0) g.draw_text(ezgl::point2d(x,y-0.000001),poi_name, 100, 100); 
            else g.draw_text(ezgl::point2d(x,y+0.000001),poi_name, 100, 100); 
        } else if (MAP.state.scale > 300) {
            g.set_color(ezgl::SADDLE_BROWN);
            g.set_text_rotation(0);
            if (i % 2 == 0) g.draw_text(ezgl::point2d(x,y-0.0000005),poi_name, 100, 100); 
            else g.draw_text(ezgl::point2d(x,y+0.0000005),poi_name, 100, 100);
        }
    }
    
        result_ids.clear();
        result_points.clear();
       }
    }


void draw_features (ezgl::renderer &g) {
    std::map<unsigned int, std::pair<double, double>> result_ids;
    std::vector<std::pair<std::pair<double, double>, unsigned int>> result_points;
    
    MAP.feature_k2tree.range_query(MAP.feature_k2tree.root, // root
                         0, // depth of query
                         std::make_pair(MAP.state.current_view_x_buffered.first, MAP.state.current_view_x_buffered.second), // x-range (smaller, greater)
                         std::make_pair(MAP.state.current_view_y_buffered.first, MAP.state.current_view_y_buffered.second), // y-range (smaller, greater)
                         result_points, // results
                         result_ids,
                         MAP.state.zoom_level, 0);
    
    // fix for very viewing inside very large features
    if (result_ids.size() < 1) {
        
        result_ids.clear();
        result_points.clear();

        MAP.feature_k2tree.range_query(MAP.feature_k2tree.root, // root
                         0, // depth of query
                         std::make_pair(x_from_lon(MAP.world_values.min_lon), x_from_lon(MAP.world_values.max_lon)), // x-range (smaller, greater)
                         std::make_pair( y_from_lat(MAP.world_values.min_lat), y_from_lat(MAP.world_values.max_lat)), // y-range (smaller, greater)
                         result_points, // results
                         result_ids,
                         MAP.state.zoom_level, 0); // zoom_level
        
    }
    
    for(std::vector<unsigned int>::iterator it = MAP.permanent_features.begin(); it != MAP.permanent_features.end(); it++) {
        result_ids.insert(std::make_pair((*it), std::make_pair(0.0, 0.0)));
    }
    
    for(std::map<unsigned int, std::pair<double, double>>::iterator it = result_ids.begin(); it != result_ids.end(); it++) { 

        int i = it->first;
        
        std::vector<ezgl::point2d> feature_points;
        
        //set feature colour:
        FeatureType feature_type = getFeatureType(i);
        switch(feature_type) {
            case Unknown    : g.set_color(ezgl::NONE); break;
            case Park       : g.set_color(ezgl::PARK_GREEN); break;
            case Beach      : g.set_color(ezgl::BEACH_YELLOW); break;
            case Lake       : g.set_color(ezgl::WATER_BLUE); break;
            case River      : g.set_color(ezgl::WATER_BLUE); break;
            case Island     : g.set_color(ezgl::BACKGROUND_GREY); break;
            case Building   : g.set_color(ezgl::BUILDING_GREY); break;
            case Greenspace : g.set_color(ezgl::PARK_GREEN); break;
            case Golfcourse : g.set_color(ezgl::PARK_GREEN); break;
            case Stream     : g.set_color(ezgl::WATER_BLUE); break;
            default         : g.set_color(ezgl::NONE); break;
        }
           
        //check if a closed feature and then draw accordingly
        if (getFeaturePoint(0,i).lat() == getFeaturePoint(getFeaturePointCount(i)-1,i).lat()
            && getFeaturePoint(0,i).lon() == getFeaturePoint(getFeaturePointCount(i)-1,i).lon()) {
        
                for (int j = 0; j < getFeaturePointCount(i); j++) {
                    double x = x_from_lon(getFeaturePoint(j, i).lon());
                    double y = y_from_lat(getFeaturePoint(j, i).lat());

                    feature_points.push_back(ezgl::point2d(x,y));
                }
                
                if(feature_points.size() > 1)
                    g.fill_poly(feature_points);
        } else {
            std::vector<LatLon> points;
            for (int point = 0; point < getFeaturePointCount(i); point ++) {
                points.push_back(getFeaturePoint(point, i));
            }
            draw_curve(g, points);
        }
    }
    
    result_ids.clear();
    result_points.clear();
}


void draw_subway_data(ezgl::renderer &g){
    g.set_color(ezgl::PURPLE); 
    g.set_line_width(3);
    
    ezgl::surface *subway_png = g.load_png("./libstreetmap/resources/Icons/subway.png");
            
    for(unsigned i = 0; i < MAP.OSM_data.subway_routes.size(); i++) {
        //draw the tracks
        for(unsigned j = 0; j < MAP.OSM_data.subway_routes[i].path.size(); j++) {
            draw_curve(g, MAP.OSM_data.subway_routes[i].path[j]);
        }
        
        //draw stations if zoomed in enough
        if(MAP.state.zoom_level >= 2) {            
            for(unsigned j =0; j < MAP.OSM_data.subway_routes[i].stations.size(); j++) {
                g.draw_surface(subway_png, png_draw_center_point(g, MAP.OSM_data.subway_routes[i].stations[j], 48));
            }
        }
    }
    
    g.free_surface(subway_png);
}

void draw_bike_data(ezgl::renderer &g) {
    g.set_color(ezgl::BIKE_GREEN);
    g.set_line_width(1);
    
    ezgl::surface *parking_png = g.load_png("./libstreetmap/resources/Icons/bike_parking.png");
    
    //draw bike paths
    for(unsigned i = 0; i < MAP.OSM_data.bike_routes.size(); i++) {
        draw_curve(g, MAP.OSM_data.bike_routes[i]);
    }
    
    //skip over some bike parking depending on zoom level to prevent overcrowding
    int skip_factor;
    switch(MAP.state.zoom_level) {
        case 0: skip_factor = MAP.OSM_data.bike_parking.size() / 25 ; break;
        case 1: skip_factor = MAP.OSM_data.bike_parking.size() / 100; break;
        case 2: skip_factor = MAP.OSM_data.bike_parking.size() / 250; break;
        case 3: skip_factor = MAP.OSM_data.bike_parking.size() / 1000; break;
        default: skip_factor = 1;
    }
    if(skip_factor < 1) skip_factor = 1;
    //draw bike parking
    for(unsigned i = 0; i < MAP.OSM_data.bike_parking.size(); i+= skip_factor) {       
        g.draw_surface(parking_png, png_draw_center_point(g, MAP.OSM_data.bike_parking[i], 16));
    }
    
    g.free_surface(parking_png);
}


//helper function to draw curves from LatLon point vector
void draw_curve(ezgl::renderer &g, std::vector<LatLon> &points) {
    for(unsigned int i = 0; i < points.size() - 1; i++) {
        double x1 = x_from_lon(points[i].lon());
        double y1 = y_from_lat(points[i].lat());
        double x2 = x_from_lon(points[i+1].lon());
        double y2 = y_from_lat(points[i+1].lat());

        ezgl::point2d start(x1,y1);
        ezgl::point2d end(x2,y2);   
        
        g.draw_line(start, end);
    }
}

//helper function to draw curves from a point2d vector
void draw_curve(ezgl::renderer &g, std::vector<ezgl::point2d> &points) {
    if(points.size() < 2 ) return;
    
    for(unsigned int i = 0; i < points.size() - 1; i++) {
        g.draw_line(points[i], points[i+1]);
    }
}


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

//shortcut keys for easy navigation
void act_on_key_press(ezgl::application *app, GdkEventKey *event, char *key_name) {
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
            std::stringstream ss(text);
            std::string entry1, entry2;
            ss >> entry1;
            ss >> entry2;
            std::cout << entry1 << " " << entry2 << std::endl;
            //while ((entry2 != "@") && (!entry2.empty())) ss >> entry2;
            if (entry2 == "&") ss >> entry1;
            std::vector<unsigned> result;
            if (!entry1.empty()) {
                if(check_and_switch_map(app, text)) {
                   return; 
                } else {
                    result = find_street_ids_from_partial_street_name(entry1);
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
                    
                    if(i >= result.size()) {
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
    for (int i = 0; i < streetID1.size() - 1; i++) {
        for (int j = 0; j < streetID2.size() - 1; j++) {
            current_intersection = find_intersection_ids_from_street_ids(streetID1[i], streetID2[j]);
            intersectionID.insert(intersectionID.end(), current_intersection.begin(), current_intersection.end()); // Combine two vectors
            //std::cout << getStreetName(streetID1[i]) << getStreetName(streetID2[j]) << std::endl;  
        }
        
    }
    MAP.state.intersection_search_result = intersectionID;
    for (int i = 0; i < intersectionID.size(); i++) {
        std::cout << getIntersectionName(intersectionID[i]) << std::endl;
    }
    //MAP.state.last_selected_intersection = intersectionID[0];
}

gboolean ezgl::press_find(GtkWidget *widget, gpointer data) {
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
    
   
    if (MAP.state.search_index == MAP.state.intersection_search_result.size()) MAP.state.search_index = 0;
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
        ezgl:zoom_fit(canvas, view);
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

void act_on_suggested_clicked(ezgl::application *app, std::string suggestion) {
    GtkEntry* text_entry = (GtkEntry *) app->get_object("SearchBar");
    std::string text = gtk_entry_get_text(text_entry);
    // Place in different formats depending whether it is the first or second street-
    if (text.find('&') == std::string::npos) suggestion = suggestion + " & ";
    else {
        suggestion = text.substr(0,text.find('&')) + "& " + suggestion;
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