/*
 * This file contains functions to draw objects to the main canvas
 */

#include "map_db.h"
#include "helper_functions.h"
#include "m1.h"
#include "m2_draw.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"


// Draws the last highlighted intersection stored in MAP
void draw_selected_intersection (ezgl::renderer &g) {
    int id = MAP.state.last_selected_intersection;
    if (id <= getNumIntersections()) {
        ezgl::surface *search_png = g.load_png("./libstreetmap/resources/search.png");
        
        float x = x_from_lon(MAP.intersection_db[id].position.lon());
        float y = y_from_lat(MAP.intersection_db[id].position.lat());
        
        g.draw_surface(search_png, png_draw_center_point(g, ezgl::point2d(x,y), 36));
        
        g.free_surface(search_png);
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
            g.set_line_width(MAP.LocalStreetSegments[id].street_segment_speed_limit / 10);
        } else if (MAP.state.zoom_level == 0) {
            g.set_line_width(MAP.LocalStreetSegments[id].street_segment_speed_limit / 30);
        } else {
            g.set_line_width(MAP.LocalStreetSegments[id].street_segment_speed_limit / 20);
        }
        
        //make highways orange
        if(MAP.LocalStreetSegments[id].importance_level < 0) {
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
                g.draw_text(mid, ">                                                                              >", distance_from_points(x1, y1, x2, y2) / 2, 100);
                g.draw_text(mid, ">                                                         >", distance_from_points(x1, y1, x2, y2) / 2, 100);
                g.draw_text(mid, ">                               >", distance_from_points(x1, y1, x2, y2) / 2, 100);
                g.draw_text(mid, ">", distance_from_points(x1, y1, x2, y2) / 2, 100);
            }
        }
    }
    
    result_ids.clear();
    result_points.clear();
}

// Draw the street segments of global route twice to draw a border around the route
void draw_route (ezgl::renderer &g) {
    // Loop over global route segments, drawing each
    for(std::vector<unsigned int>::iterator it = MAP.route_data.route_segments.begin(); it != MAP.route_data.route_segments.end(); it++) { 
        
        int id = *it;
 
        g.set_color(ezgl::ROUTE_OUTLINE_BLUE);
        
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
            g.set_line_width(2 * MAP.LocalStreetSegments[id].street_segment_speed_limit / 10);
        } else if (MAP.state.zoom_level == 0) {
            g.set_line_width(2 * MAP.LocalStreetSegments[id].street_segment_speed_limit / 30);
        } else {
            g.set_line_width(2 * MAP.LocalStreetSegments[id].street_segment_speed_limit / 20);
        }
        
        draw_curve(g, points);
    }
    
    for(std::vector<unsigned int>::iterator it = MAP.route_data.route_segments.begin(); it != MAP.route_data.route_segments.end(); it++) { 
        
        int id = *it;
 
        g.set_color(ezgl::ROUTE_BLUE);
        
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
            g.set_line_width(1 * MAP.LocalStreetSegments[id].street_segment_speed_limit / 10);
        } else if (MAP.state.zoom_level == 0) {
            g.set_line_width(1 * MAP.LocalStreetSegments[id].street_segment_speed_limit / 30);
        } else {
            g.set_line_width(1 * MAP.LocalStreetSegments[id].street_segment_speed_limit / 20);
        }
        
        draw_curve(g, points);
    }
}

// Draw the start/end markers for global route
void draw_route_start_end (ezgl::renderer &g) {
    // Draw start marker
    if(MAP.route_data.start_intersection < MAP.intersection_db.size()) {
        ezgl::surface *start_png = g.load_png("./libstreetmap/resources/GreenLocationMarkerDouble.png");
        double x = x_from_lon(getIntersectionPosition(MAP.route_data.start_intersection).lon());
        double y = y_from_lat(getIntersectionPosition(MAP.route_data.start_intersection).lat());
        g.draw_surface(start_png, png_draw_bottom_middle(g, ezgl::point2d(x,y), 48, 30));
    }
    // Draw end marker
    if(MAP.route_data.end_intersection < MAP.intersection_db.size()) {
        ezgl::surface *end_png = g.load_png("./libstreetmap/resources/BlueLocationMarkerDouble.png");
        double x2 = x_from_lon(getIntersectionPosition(MAP.route_data.end_intersection).lon());
        double y2 = y_from_lat(getIntersectionPosition(MAP.route_data.end_intersection).lat());
        g.draw_surface(end_png, png_draw_bottom_middle(g, ezgl::point2d(x2,y2), 48, 30));
    }
}


// Uses a range query based on the current view and zoom level to find all the 
// street segments for this view and draw them
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
                ezgl::point2d point_1 = point2d_from_LatLon(*it_p);
                ezgl::point2d point_2 = point2d_from_LatLon(*(it_p+1));
                
                double angle = angle_from_2_point2d(point_1, point_2);
                
                //keep orientation of text the same
                if (angle > 90) angle = angle - 180;
                else if (angle < -90) angle = angle + 180;
                
                g.set_color(ezgl::BLACK);
                
                ezgl::point2d mid((point_2.x + point_1.x) / 2.0, (point_2.y + point_1.y) / 2.0);
                
                
                g.set_text_rotation(angle);
                g.draw_text(mid, getStreetName(getInfoStreetSegment(i).streetID), 
                        distance_from_points(point_1.x, point_1.y, point_2.x, point_2.y), 100);
            }
        }
        points.clear();
    }
    
    result_ids.clear();
    result_points.clear();
}


// Uses a range query based on the current view and zoom level to find
// the POIs only up to a calculated search_depth, which declusters the POIs
// by only going a certain depth into the k2tree
// Also draws street names
void draw_points_of_interest (ezgl::renderer &g) {
    std::map<unsigned int, std::pair<double, double>> result_ids;
    std::vector<std::pair<std::pair<double, double>, unsigned int>> result_points;
    
    ezgl::surface *poi_png = g.load_png("./libstreetmap/resources/IntersectionIcon.png");
    // Decide what level of POI detail to show based on zoom level
    std::size_t search_depth = 0;
    if(MAP.state.zoom_level > 3) {
        if(MAP.state.current_width >= 500) search_depth = 10;
        else search_depth = 0;
    } else if(MAP.state.zoom_level > 2) {
        search_depth = 8;
    }  else if(MAP.state.zoom_level > 1) {
        search_depth = 6;
    } else  if(MAP.state.zoom_level > 0) {
        search_depth = 4;
    } else {
        search_depth = 1;
    }
    MAP.poi_k2tree.range_query(MAP.poi_k2tree.root, // root
                         0, // depth of query
                         std::make_pair(MAP.state.current_view_x_buffered.first, MAP.state.current_view_x_buffered.second), // x-range (smaller, greater)
                         std::make_pair(MAP.state.current_view_y_buffered.first, MAP.state.current_view_y_buffered.second), // y-range (smaller, greater)
                         result_points, // results
                         result_ids,
                         MAP.state.zoom_level, search_depth); // zoom_level
            
    for(std::map<unsigned int, std::pair<double, double>>::iterator it = result_ids.begin(); it != result_ids.end(); it++) { 
        
        int i = it->first;

        double x = x_from_lon(getPointOfInterestPosition(i).lon());
        double y = y_from_lat(getPointOfInterestPosition(i).lat());
        std::string poi_name = getPointOfInterestName(i);

        g.draw_surface(poi_png, png_draw_center_point(g, ezgl::point2d(x,y), 24));

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
    
    g.free_surface(poi_png);
}


// Uses a range query to find the features that have points in the current view,
// if no features are found, sets range to max bounds (for inside very large features),
// and also adds all permanent features, ie features that intersect the outerbounds.
// The permanent features account for large features such as an ocean surrounding an island.
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


// Draws subway data for all of the routes and  subway stops. Subways stops
// are only drawn above zoom level 2. This does not use KD Trees as subway routes
// are generally smaller data sets and stops are only drawn above zoom level 2.
void draw_subway_data(ezgl::renderer &g){
    g.set_color(ezgl::PURPLE); 
    g.set_line_width(3);
    
    ezgl::surface *subway_png = g.load_png("./libstreetmap/resources/subway.png");
            
    for(unsigned i = 0; i < MAP.OSM_data.subway_routes.size(); i++) {
        //draw the tracks
        for(unsigned j = 0; j < MAP.OSM_data.subway_routes[i].path.size(); j++) {
            draw_curve(g, MAP.OSM_data.subway_routes[i].path[j]);
        }
        
        //draw stations if zoomed in enough
        if(MAP.state.zoom_level >= 2) {            
            for(unsigned j =0; j < MAP.OSM_data.subway_routes[i].stations.size(); j++) {
                g.draw_surface(subway_png, png_draw_center_point(g, MAP.OSM_data.subway_routes[i].stations[j], 24));
            }
        }
    }
    
    g.free_surface(subway_png);
}

// Draws bike data including all bike paths and some of the bike parking, depending
// on the zoom_level. Does not use KD2Tree since this is generally a smaller
// dataset.
void draw_bike_data(ezgl::renderer &g) {
    g.set_color(ezgl::BIKE_GREEN);
    g.set_line_width(1);
    
    ezgl::surface *parking_png = g.load_png("./libstreetmap/resources/bike_parking.png");
    
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
