/*
 * Initial graphics implementation
 */

#include "map_db.h"
#include "helper_functions.h"
#include "m1.h"
#include "m2.h"
#include "m2_draw.h"
#include "m2_callbacks.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"

void draw_map () {
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
    
    //Convert latitude and longitude into x and y coordinates 
    ezgl::rectangle initial_world(
            {x_from_lon(MAP.world_values.min_lon), y_from_lat(MAP.world_values.min_lat)}, 
            {x_from_lon(MAP.world_values.max_lon), y_from_lat(MAP.world_values.max_lat)}
    );
    
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);
    
    
    application.run(nullptr, act_on_mouse_click, 
                    nullptr, act_on_key_press,
                    act_on_transit_toggle, act_on_bikes_toggle,
                    act_on_poi_toggle, act_on_suggested_clicked,
                    act_on_find, act_on_directions);
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
    
    if (MAP.state.current_width >= 50000)       MAP.state.zoom_level = -1;
    else if (MAP.state.current_width >= 20000)  MAP.state.zoom_level =  0;
    else if (MAP.state.current_width  >= 7500)  MAP.state.zoom_level =  1;
    else if (MAP.state.current_width >= 3000)   MAP.state.zoom_level =  2;
    else if (MAP.state.current_width >= 2000)   MAP.state.zoom_level =  3;
    else                                        MAP.state.zoom_level =  4;
        
    g.set_color(ezgl::BACKGROUND_GREY);
    
    g.fill_rectangle(
            {MAP.state.current_view_x.first, MAP.state.current_view_y.first}, 
            {MAP.state.current_view_x.second, MAP.state.current_view_y.second}
    );
    
    g.set_line_cap(ezgl::line_cap::round);
    draw_features(g);
    draw_street_segments(g);
    if(MAP.state.is_bikes_on) draw_bike_data(g);
    draw_selected_intersection(g);
    if(MAP.state.is_transit_on) draw_subway_data(g);
    draw_route(g);
    draw_street_name(g);
    if(MAP.state.is_poi_on) draw_points_of_interest(g);
    draw_route_start_end(g);
}
