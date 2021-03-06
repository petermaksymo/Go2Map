/*
 * A implementation of path finding and function that helps with its functionality
 */

#include <m1.h>
#include <m3.h>
#include <constants.hpp>
#include <StreetsDatabaseAPI.h>
#include <math.h>
#include "helper_functions.h"
#include <cmath>
#include <stdio.h>
#include <map_db.h>
#include <vector>
#include <bits/stdc++.h>

ezgl::point2d get_other_segment_point(int intersection_id, InfoStreetSegment & segment, StreetSegmentIndex segment_id);


// Forward declaration of functions
bool bfsPath(Node* sourceNode, int destID, double right_turn_penalty, double left_turn_penalty);
std::vector<unsigned>& backtrace(std::vector<unsigned>& route, int intersect_id_start, int intersect_id_end);

TurnType find_turn_type(unsigned segment1_id, unsigned segment2_id) {
    InfoStreetSegment segment1 = getInfoStreetSegment(segment1_id);
    InfoStreetSegment segment2 = getInfoStreetSegment(segment2_id);
    
    int intersection_id = 0;
    
    // Find the intersection the turn takes place, returns NONE if not found
    if (segment1.to == segment2.to) intersection_id = segment1.to; 
    else if (segment1.to == segment2.from) intersection_id = segment1.to; 
    else if (segment1.from == segment2.from) intersection_id = segment1.from; 
    else if (segment1.from == segment2.to) intersection_id = segment1.from;  
    else return TurnType::NONE;
    
    // Return STRAIGHT if streets have same id
    if (segment1.streetID == segment2.streetID) return TurnType::STRAIGHT;
    
    // get the points for calculation
    ezgl::point2d seg1_point = get_other_segment_point(intersection_id, segment1, segment1_id);
    ezgl::point2d seg2_point = get_other_segment_point(intersection_id, segment2, segment2_id);
    ezgl::point2d intersection_point = point2d_from_LatLon(getIntersectionPosition(intersection_id));
    
    //convert to 2 vectors
    ezgl::point2d vec1 = intersection_point - seg1_point;
    ezgl::point2d vec2 = seg2_point - intersection_point;
    
    //find angle between the two vectors
    //method/idea from: https://stackoverflow.com/a/21486462
    //double dot = vec1.x * vec2.x + vec1.y * vec2.y; //dot product
    double det = vec1.x * vec2.y - vec1.y * vec2.x; //determinant
    //double angle = atan2(det, dot);
    
    //optimized to decide turn type based off determinant, left old angle
    //code commented in case we want the angle in the future
    if(det > 0) return TurnType::LEFT;
    return TurnType::RIGHT;
}


//Helper function that will return the first point in a street segment (curve point or intersection point)
//that is closest to the intersection of interest
ezgl::point2d get_other_segment_point(int intersection_id, InfoStreetSegment & segment, StreetSegmentIndex segment_id) {
    LatLon point_LL;
    
    //if no curve points, the other point is location of other intersection
    if(segment.curvePointCount == 0) {
        point_LL = segment.to == intersection_id
            ? getIntersectionPosition(segment.from)
            : getIntersectionPosition(segment.to);
    } else {
        point_LL = segment.to == intersection_id
            ? getStreetSegmentCurvePoint(segment.curvePointCount-1, segment_id)
            : getStreetSegmentCurvePoint(0, segment_id);
    }
    
    return point2d_from_LatLon(point_LL);
}


double compute_path_travel_time(const std::vector<unsigned>& path, 
                                const double right_turn_penalty, 
                                const double left_turn_penalty) {
    double travel_time = 0.0;
    
    //add path time, then check for turn and add turn penalty
    if (path.size() > 1) {
            for (auto it = path.begin(); it != path.end() - 1; ++it) {
                travel_time += MAP.LocalStreetSegments[*it].travel_time; //find_street_segment_travel_time(*it);

                TurnType turn = find_turn_type(*it, *(it + 1));
                switch (turn) {
                    case TurnType::LEFT: travel_time += left_turn_penalty;
                        break;
                    case TurnType::RIGHT: travel_time += right_turn_penalty;
                        break;
                    default: break;
                }
            }
            //add last path (not done in for loop))
            travel_time += MAP.LocalStreetSegments[path[path.size() - 1]].travel_time;
    } else {
        // In case this is no path existing at all
        if (path.size() == 1) travel_time = MAP.LocalStreetSegments[path[0]].travel_time;
    }
    return travel_time;
}


std::vector<unsigned> find_path_between_intersections(
		  const unsigned intersect_id_start, 
                  const unsigned intersect_id_end,
                  const double right_turn_penalty, 
                  const double left_turn_penalty) {
    std::vector<unsigned> route;
    if (bfsPath(MAP.intersection_node[intersect_id_start], intersect_id_end, right_turn_penalty, left_turn_penalty)) {
        return backtrace(route, intersect_id_start, intersect_id_end);
    }
    else {
        clear_intersection_node();
        std::cout<< "no route found\n";
        return route;
    }
}


bool bfsPath(Node* sourceNode, int destID, double right_turn_penalty, double left_turn_penalty) {
    // Initialize queue for BFS
    std::priority_queue <waveElem, std::vector<waveElem>, comparator> wavefront; 
   
    // Queue the source node 
    waveElem sourceElem = waveElem(sourceNode, NO_EDGE, 0.0); 
    
    //std::cout << (sourceElem.node)->intersection_id << std::endl;
    wavefront.push(sourceElem); 
   
    // Do bfs while the wavefront is not empty
    while (!wavefront.empty()) {
        waveElem currentElem = wavefront.top(); // Fetch the first item from the wavefront
        
        wavefront.pop(); // Remove the first element
        Node* currentNode = currentElem.node;
        
        // Check every node that is connected to the current node
        for (unsigned i = 0; i < currentNode->edge_out.size(); i++) {
            int currentEdge = currentNode->edge_out[i];
            InfoStreetSegment edgeInfo = getInfoStreetSegment(currentEdge);
            double travel_time = MAP.LocalStreetSegments[currentEdge].travel_time;
            Node* nextNode;
            double turn_penalty = 0;
            
            // Assign the next node of the current edge
            // If state prevents going back to previous node or going through one way street
            if (currentEdge!= currentNode->edge_in && !(edgeInfo.oneWay && (currentNode->intersection_id == edgeInfo.to))) {
                
                // Determine the next node that is connected to the current searching edge
                nextNode = (edgeInfo.from == currentNode->intersection_id)
                    ? MAP.intersection_node[edgeInfo.to]
                    : MAP.intersection_node[edgeInfo.from];
                // Determine turn penalty base on turn type
                if (currentNode->edge_in != NO_EDGE) {
                   if (find_turn_type(currentNode->edge_in, currentEdge) == TurnType::LEFT) turn_penalty = left_turn_penalty;
                   else if (find_turn_type(currentNode->edge_in, currentEdge) == TurnType::RIGHT) turn_penalty = right_turn_penalty;
                   else turn_penalty = 0; 
                }
                
                    
            // Only update the node data and wavefront if it is a faster solution or it was never reached before
            if (nextNode->best_time == 0 || currentNode->best_time + travel_time + turn_penalty < nextNode->best_time) {
                
                nextNode->edge_in = currentEdge;          
                
                nextNode->best_time = currentNode->best_time + travel_time + turn_penalty;
                
                // Queue the new wave element with newly approximated travel_time
                wavefront.push(waveElem(nextNode, currentEdge, currentNode->best_time 
                        + travel_time + turn_penalty + find_distance_between_two_points(
                        getIntersectionPosition(nextNode->intersection_id), getIntersectionPosition(destID)) / 29.1));
                }
            }
        }
        if (currentNode->intersection_id == destID) {return true; }
        
    } 
    
    return false;
}

// Function that backtraces the best route
std::vector<unsigned>& backtrace(std::vector<unsigned>& route, int intersect_id_start, int intersect_id_end) { 
    Node* currentNode = MAP.intersection_node[intersect_id_end];
    
    // Follow the best route from destination to start
    while (currentNode->intersection_id != intersect_id_start) {
        int prevEdge = currentNode->edge_in;
        route.insert(route.begin(), prevEdge);
        InfoStreetSegment edgeInfo = getInfoStreetSegment(prevEdge);
        currentNode = (edgeInfo.from == currentNode->intersection_id)
            ? MAP.intersection_node[edgeInfo.to]
            : MAP.intersection_node[edgeInfo.from];  
    }
    
    // Store all the data to MAP for display
    for (auto it = route.begin(); it != route.end(); it++) {
        MAP.route_data.route_segments.push_back(*it);
    }
    
    //MAP.route_data.route_segments.push_back(142146);
    MAP.route_data.start_intersection = intersect_id_start;
    MAP.route_data.end_intersection = intersect_id_end;
    clear_intersection_node();
    return route;
}
