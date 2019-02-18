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
void draw_street_segments (ezgl::renderer &g);
void draw_points_of_interest (ezgl::renderer &g);
void draw_features (ezgl::renderer &g);

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
    
    draw_features(g);
    draw_street_segments(g);
    draw_points_of_interest(g);
    draw_intersections(g);
}


void draw_intersections (ezgl::renderer &g) {
    for (unsigned int i = 0; i < MAP.intersection_db.size(); i++) {
        float x = x_from_lon(MAP.intersection_db[i].position.lon());
        float y = y_from_lat(MAP.intersection_db[i].position.lat());
        
        float width = (x_from_lon(MAP.world_values.max_lon) - x_from_lon(MAP.world_values.min_lon))/2000;
        float height = width;
        
        //only draw selected intersections
        if (MAP.intersection_db[i].is_selected) {
            g.set_color(ezgl::RED);
            g.fill_rectangle({x,y}, {x+width, y+height});
        } 
        
    }
}


void draw_street_segments (ezgl::renderer &g) {
    g.set_color(ezgl::GREY_55);
    
    for (unsigned int i = 0; i < unsigned(getNumStreetSegments()); i++) {
        double x1 = x_from_lon(MAP.intersection_db[getInfoStreetSegment(i).from].position.lon());
        double y1 = y_from_lat(MAP.intersection_db[getInfoStreetSegment(i).from].position.lat());
        double x2 = x_from_lon(MAP.intersection_db[getInfoStreetSegment(i).to].position.lon());
        double y2 = y_from_lat(MAP.intersection_db[getInfoStreetSegment(i).to].position.lat());
        
        ezgl::point2d start(x1,y1);
        ezgl::point2d end(x2,y2);   
        
        g.draw_line(start, end);
    }
}


void draw_points_of_interest (ezgl::renderer &g) {
    for (unsigned int i = 0; i < unsigned(getNumPointsOfInterest()); i++) {
        double x = x_from_lon(getPointOfInterestPosition(i).lon());
        double y = y_from_lat(getPointOfInterestPosition(i).lat());
        
        float width = (x_from_lon(MAP.world_values.max_lon) - x_from_lon(MAP.world_values.min_lon))/2000;
        float height = width;
        
        g.set_color(ezgl::GREEN);
        
        g.fill_rectangle({x,y}, {x+width, y+height});
    }
}


void draw_features (ezgl::renderer &g) {
    for (unsigned int i = 0; i < unsigned(getNumFeatures()); i++) {
        std::vector<ezgl::point2d> feature_points;
        g.set_color(ezgl::BLUE);
           
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
            for (int point = 0; point < getFeaturePointCount(i) - 1; point ++) {
                double x1 = x_from_lon(getFeaturePoint(point, i).lon());
                double y1 = y_from_lat(getFeaturePoint(point, i).lat());
                double x2 = x_from_lon(getFeaturePoint(point+1, i).lon());
                double y2 = y_from_lat(getFeaturePoint(point+1, i).lat());

                ezgl::point2d start(x1,y1);
                ezgl::point2d end(x2,y2);   

                g.draw_line(start, end);
            }
        }
    }
    
    
}


//currently highlights closest intersection red
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y) {
    //clear previously selected intersection
    if(MAP.last_selected_intersection <= getNumIntersections()) {
        MAP.intersection_db[MAP.last_selected_intersection].is_selected = false;
    }
    
    std::cout << "Mouse clicked at (" << x << ", " << y << ")\n";
    
    LatLon position = LatLon(lat_from_y(y), lon_from_x(x));
    int id = find_closest_intersection(position);
    
    std::cout << "Closest Intersection: " << MAP.intersection_db[id].name << "\n";
    MAP.intersection_db[id].is_selected = true;
    MAP.last_selected_intersection = id;
    
    app->refresh_drawing();
}