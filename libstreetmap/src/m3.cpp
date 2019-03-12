/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <m1.h>
#include <m3.h>
#include <StreetsDatabaseAPI.h>
#include <math.h>
#include "helper_functions.h"

TurnType find_turn_type(unsigned street_segment1, unsigned street_segment2) {
    InfoStreetSegment segment1 = getInfoStreetSegment(street_segment1);
    InfoStreetSegment segment2 = getInfoStreetSegment(street_segment2);
    
    double x1 = x_from_lon(getIntersectionPosition(segment1.from).lon());
    double y1 = y_from_lat(getIntersectionPosition(segment1.from).lat());
    double x2 = x_from_lon(getIntersectionPosition(segment1.to).lon());
    double y2 = y_from_lat(getIntersectionPosition(segment1.to).lat());   
    double slope_segment1 = (atan( (y2-y1)/(x2-x1) ) )/ DEG_TO_RAD;
    
    double x3 = x_from_lon(getIntersectionPosition(segment2.from).lon());
    double y3 = y_from_lat(getIntersectionPosition(segment2.from).lat());
    double x4 = x_from_lon(getIntersectionPosition(segment2.to).lon());
    double y4 = y_from_lat(getIntersectionPosition(segment2.to).lat());   
    double slope_segment2 = (atan( (y4-y3)/(x4-x3))) / DEG_TO_RAD;
    
    // Return NONE if streets have same id
    if (street_segment1 == street_segment2) return TurnType::NONE;
    
    // Return NONE if streets don't intersect
    if (segment1.to != segment2.to|| 
        segment1.to != segment2.from||
        segment1.from != segment2.from||
        segment1.from != segment2.to  
            ) return TurnType::NONE;
    
    if (slope_segment1 >= slope_segment2) return TurnType::RIGHT;
    if (slope_segment1 < slope_segment2) return TurnType::LEFT;
    
    
    
    
    
    
}

double compute_path_travel_time(const std::vector<unsigned>& path, 
                                const double right_turn_penalty, 
                                const double left_turn_penalty) {
    
}

std::vector<unsigned> find_path_between_intersections(
		  const unsigned intersect_id_start, 
                  const unsigned intersect_id_end,
                  const double right_turn_penalty, 
                  const double left_turn_penalty) {
    
}