/*
 * Initial graphics implementation (may need to be refactored to separate files later)
 */

#include "map_db.h"
#include "helper_functions.h"
#include "m2.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"

void draw_main_canvas (ezgl::renderer &g);
void draw_intersections (ezgl::renderer &g);

void draw_map () {
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
    
    ezgl::rectangle initial_world(
            {lon_to_x(MAP.world_values.min_lon), lat_to_y(MAP.world_values.min_lat)}, 
            {lon_to_x(MAP.world_values.max_lon), lat_to_y(MAP.world_values.max_lat)}
    );
    
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);
    
    
    application.run(nullptr, nullptr, nullptr, nullptr);
}

void draw_main_canvas (ezgl::renderer &g) {    
    g.draw_rectangle(
            {lon_to_x(MAP.world_values.min_lon), lat_to_y(MAP.world_values.min_lat)}, 
            {lon_to_x(MAP.world_values.max_lon), lat_to_y(MAP.world_values.max_lat)}
    );
    
    draw_intersections(g);
}

void draw_intersections (ezgl::renderer &g) {
    for (unsigned int i = 0; i < MAP.intersection_db.size(); i++) {
        float x = lon_to_x(MAP.intersection_db[i].position.lon());
        float y = lat_to_y(MAP.intersection_db[i].position.lat());
        
        float width = (lon_to_x(MAP.world_values.max_lon) - lon_to_x(MAP.world_values.min_lon))/1000;
        float height = width;
        
        g.fill_rectangle({x,y}, {x+width, y+height});
    }
}