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
void draw_selected_intersection (ezgl::renderer &g);
void draw_street_segments (ezgl::renderer &g);
void draw_points_of_interest (ezgl::renderer &g);
void draw_features (ezgl::renderer &g);
void draw_street_name(ezgl::renderer &g);
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y);
void act_on_mouse_move(ezgl::application *app, GdkEventButton *event, double x, double y);


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
    
    
    application.run(nullptr, act_on_mouse_click, act_on_mouse_move, nullptr);
}


void draw_main_canvas (ezgl::renderer &g) {    
    g.set_color(ezgl::BACKGROUND_GREY);
    g.fill_rectangle(
            {x_from_lon(MAP.world_values.min_lon), y_from_lat(MAP.world_values.min_lat)}, 
            {x_from_lon(MAP.world_values.max_lon), y_from_lat(MAP.world_values.max_lat)}
    );
    
    draw_features(g);
    draw_street_segments(g);
    draw_points_of_interest(g);
    draw_selected_intersection(g);
    draw_street_name(g);
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


void draw_street_segments (ezgl::renderer &g) {
    g.set_color(ezgl::WHITE);
    
    for (unsigned int i = 0; i < unsigned(getNumStreetSegments()); i++) {
        double x1 = x_from_lon(MAP.intersection_db[getInfoStreetSegment(i).from].position.lon());
        double y1 = y_from_lat(MAP.intersection_db[getInfoStreetSegment(i).from].position.lat());
        double x2 = x_from_lon(MAP.intersection_db[getInfoStreetSegment(i).to].position.lon());
        double y2 = y_from_lat(MAP.intersection_db[getInfoStreetSegment(i).to].position.lat());
        
        ezgl::point2d start(x1,y1);
        ezgl::point2d end(x2,y2);   
        
        g.set_line_width(MAP.LocalStreetSegments.street_segment_speed_limit[i] / 20);
        
        g.draw_line(start, end);
    }
}

void draw_street_name(ezgl::renderer &g) {
    for (unsigned int i = 0; i < unsigned(getNumStreetSegments()); i++) {
        double x1 = x_from_lon(MAP.intersection_db[getInfoStreetSegment(i).from].position.lon());
        double y1 = y_from_lat(MAP.intersection_db[getInfoStreetSegment(i).from].position.lat());
        double x2 = x_from_lon(MAP.intersection_db[getInfoStreetSegment(i).to].position.lon());
        double y2 = y_from_lat(MAP.intersection_db[getInfoStreetSegment(i).to].position.lat());
        
        g.set_color(ezgl::RED);
        ezgl::point2d mid((x2 + x1) / 2.0, (y2 + y1) / 2.0);     
        ezgl::rectangle current_view = g.get_visible_world();
        
        double scale = (x_from_lon(MAP.world_values.max_lon) - x_from_lon(MAP.world_values.min_lon)) / 
        (current_view.right() - current_view.left());
        //std::cout << scale << std::endl;
        if (scale > 58 && i % 3 == 0 && getStreetName(getInfoStreetSegment(i).streetID) != "<unknown>") {
            g.draw_text(mid, getStreetName(getInfoStreetSegment(i).streetID));
        }
    }
}

void draw_points_of_interest (ezgl::renderer &g) {
    for (unsigned int i = 0; i < unsigned(getNumPointsOfInterest()); i++) {
        double x = x_from_lon(getPointOfInterestPosition(i).lon());
        double y = y_from_lat(getPointOfInterestPosition(i).lat());
        
        float radius = (x_from_lon(MAP.world_values.max_lon) - x_from_lon(MAP.world_values.min_lon))/7500;
        
        g.set_color(ezgl::RED);       
        g.fill_arc(ezgl::point2d(x,y), radius, 0, 360);;
    }
}


void draw_features (ezgl::renderer &g) {
    for (unsigned int i = 0; i < unsigned(getNumFeatures()); i++) {
        std::vector<ezgl::point2d> feature_points;
        
        //set feature colour:
        FeatureType feature_type = getFeatureType(i);
        switch(feature_type) {
            case Unknown    : g.set_color(ezgl::NONE); break;
            case Park       : g.set_color(ezgl::PARK_GREEN); break;
            case Beach      : g.set_color(ezgl::BEACH_YELLOW); break;
            case Lake       : g.set_color(ezgl::WATER_BLUE); break;
            case River      : g.set_color(ezgl::WATER_BLUE); break;
            case Island     : g.set_color(ezgl::PARK_GREEN); break;
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

        std::cout << "Closest Intersection: " << MAP.intersection_db[id].name << "\n";
        MAP.intersection_db[id].is_selected = true;
        MAP.state.last_selected_intersection = id;

        app->refresh_drawing();
        //end of use from tutorial slides
    } 
}

void act_on_mouse_move(ezgl::application *app, GdkEventButton *event, double x, double y) {        
  
}