/*
 * Initial graphics implementation (may need to be refactored to separate files later)
 */

#include "map_db.h"
#include "helper_functions.h"
#include "m1.h"
#include "m2.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"

void draw_main_canvas (ezgl::renderer &g);
void draw_intersections (ezgl::renderer &g);
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y);


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
    
    
    application.run(nullptr, act_on_mouse_click, nullptr, nullptr);
}


void draw_main_canvas (ezgl::renderer &g) {    
    g.draw_rectangle(
            {x_from_lon(MAP.world_values.min_lon), y_from_lat(MAP.world_values.min_lat)}, 
            {x_from_lon(MAP.world_values.max_lon), y_from_lat(MAP.world_values.max_lat)}
    );
    
    draw_intersections(g);
}


void draw_intersections (ezgl::renderer &g) {
    for (unsigned int i = 0; i < MAP.intersection_db.size(); i++) {
        float x = x_from_lon(MAP.intersection_db[i].position.lon());
        float y = y_from_lat(MAP.intersection_db[i].position.lat());
        
        float width = (x_from_lon(MAP.world_values.max_lon) - x_from_lon(MAP.world_values.min_lon))/1000;
        float height = width;
        
        if (MAP.intersection_db[i].selected) {
            g.set_color(ezgl::RED);
        } else {
            g.set_color(ezgl::GREY_55);
        }
        
        g.fill_rectangle({x,y}, {x+width, y+height});
    }
}

void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y) {
    std::cout << "Mouse clicked at (" << x << ", " << y << ")\n";
    
    LatLon position = LatLon(lat_from_y(y), lon_from_x(x));
    int id = find_closest_intersection(position);
    
    std::cout << "Closest Intersection: " << MAP.intersection_db[id].name << "\n";
    MAP.intersection_db[id].selected = true;
    
    app->refresh_drawing();
}