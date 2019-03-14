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
#include <cmath>

TurnType find_turn_type(unsigned street_segment1, unsigned street_segment2) {
    InfoStreetSegment segment1 = getInfoStreetSegment(street_segment1);
    InfoStreetSegment segment2 = getInfoStreetSegment(street_segment2);
    
    double street1_x, street1_y, street2_x, street2_y;
    int intersection_id = 0;
    
    // Check the intersectionIndex
    if (segment1.to == segment2.to) intersection_id = segment1.to; 
    else if (segment1.to == segment2.from) intersection_id = segment1.to; 
    else if (segment1.from == segment2.from) intersection_id = segment1.from; 
    else if (segment1.from == segment2.to) intersection_id = segment1.from;  
    else return TurnType::NONE;
    
    // Return NONE if streets have same id
    if (segment1.streetID == segment2.streetID) return TurnType::STRAIGHT;
    
    // Calculate the slope of first street segment
    // If the street has no curvepoint, then the other end is the end point
    if (segment1.curvePointCount == 0) {
        street1_x = x_from_lon(getIntersectionPosition(segment1.to).lon());
        street1_y = y_from_lat(getIntersectionPosition(segment1.to).lat());
    }
    // If street1 has 'from' end as intersection, then first curvepoint is the end point for calculation
    else if (segment1.from == intersection_id) { 
        street1_x = x_from_lon(getStreetSegmentCurvePoint(0, street_segment1).lon());
        street1_y = y_from_lat(getStreetSegmentCurvePoint(0, street_segment1).lat());
    } 
    // If street1 has 'to' end as intersection, then last curvepoint is the end point for calculation
    else {
        street1_x = x_from_lon(getStreetSegmentCurvePoint(segment1.curvePointCount - 1, street_segment1).lon());
        street1_y = y_from_lat(getStreetSegmentCurvePoint(segment1.curvePointCount - 1, street_segment1).lat());
    }
    
    // Calculate the slope of second street segment
    if (segment2.curvePointCount == 0) {
        street2_x = x_from_lon(getIntersectionPosition(segment2.to).lon());
        street2_y = y_from_lat(getIntersectionPosition(segment2.to).lat());
    } else if (segment1.from == intersection_id) {
        street2_x = x_from_lon(getStreetSegmentCurvePoint(0, street_segment2).lon());
        street2_y = y_from_lat(getStreetSegmentCurvePoint(0, street_segment2).lat());
    } else {
        street2_x = x_from_lon(getStreetSegmentCurvePoint(segment1.curvePointCount - 1, street_segment2).lon());
        street2_y = y_from_lat(getStreetSegmentCurvePoint(segment1.curvePointCount - 1, street_segment2).lat());
    }
    
    // Fetch x,y position of the intersection
    double intersection_x = x_from_lon(getIntersectionPosition(intersection_id).lon());
    double intersection_y = y_from_lat(getIntersectionPosition(intersection_id).lat());
    
    // Calculate the slope of each street segment
    double slope_segment1; 
    double slope_segment2;
    if (street1_x - intersection_x == 0) slope_segment1 = 90; 
    else if (abs(street1_y - intersection_y == 0)) slope_segment1 = 0;
    else slope_segment1 = (atan(abs(street1_y - intersection_y) / abs(street1_x - intersection_x)) ) / DEG_TO_RAD;
    
    if (street2_x - intersection_x == 0) slope_segment2 = 90; 
    else if (abs(street2_y - intersection_y == 0)) slope_segment2 = 0;
    else slope_segment2 = (atan(abs(street2_y - intersection_y) / abs(street2_x - intersection_x)) ) / DEG_TO_RAD;
    
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