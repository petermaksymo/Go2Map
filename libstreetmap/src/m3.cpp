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
#include <stdio.h>
#include <queue>
#include <map_db.h>

ezgl::point2d get_other_segment_point(int intersection_id, InfoStreetSegment & segment, StreetSegmentIndex segment_id);

#define NO_EDGE -1

// A wave element to traverse through all the node
class waveElem {
    public:
        
    Node* node;
    int edgeID;
    double travel_time;
    
    waveElem(Node* node, int edgeID, double travel_time) {
        node = node;
        edgeID= edgeID;
        travel_time = travel_time;
    }
};

// A custom comparator used for sort min heap base on travel_time
class comparator { 
    public: 
    bool operator() (waveElem& point1, waveElem& point2) 
    { 
        return point1.travel_time < point2.travel_time; 
    } 
}; 
// Forward declaration of functions
bool bfsPath(Node* sourceNode, int destID);

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
    double dot = vec1.x * vec2.x + vec1.y * vec2.y; //dot product
    double det = vec1.x * vec2.y - vec1.y * vec2.x; //determinant
    double angle = atan2(det, dot);
    
    if(angle > 0) return TurnType::LEFT;
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
    for(auto it = path.begin(); it != path.end() - 1; ++it) {
        travel_time += find_street_segment_travel_time(*it);
        
        TurnType turn = find_turn_type(*it, *(it+1));
        switch(turn) {
            case TurnType::LEFT  : travel_time += left_turn_penalty;  break;
            case TurnType::RIGHT : travel_time += right_turn_penalty; break;
            default: break;
        }
        
    }
    //add last path (not done in for loop))
    travel_time += find_street_segment_travel_time(path[path.size()-1]);

    return travel_time;
}

std::vector<unsigned> find_path_between_intersections(
		  const unsigned intersect_id_start, 
                  const unsigned intersect_id_end,
                  const double right_turn_penalty, 
                  const double left_turn_penalty) {
    // Initialize the starting intersection 
    bfsPath(MAP.intersection_node[intersect_id_start], intersect_id_end);
    
}

bool bfsPath(Node* sourceNode, int destID) {
    // Initialize queue for BFS
    std::priority_queue<waveElem, std::vector<waveElem>, comparator> wavefront; 
    
    // Queue the source node 
    wavefront.push(waveElem(sourceNode, NO_EDGE, 0));
    
    // Do bfs while the wavefront is not empty
    while (!wavefront.empty()) {
        waveElem currentElem = wavefront.top(); // Fetch the first item from the wavefront
        
        wavefront.pop(); // Remove the first element
        Node* currentNode = currentElem.node;
        
        for (int i = 0; i < currentNode->edge_out.size(); i++) {
            int currentEdge = currentNode->edge_out[i];
            InfoStreetSegment edgeInfo = getInfoStreetSegment(currentEdge);
            double travel_time = MAP.LocalStreetSegments[i].travel_time;
            
            // Assign the next node of the current edge
            Node* nextNode = (edgeInfo.from == currentNode->intersection_id)
                    ? MAP.intersection_node[edgeInfo.to]
                    : MAP.intersection_node[edgeInfo.from];
            
            // Only update the node data and wavefront if it is a faster solution
            if (currentNode->best_time == 0 || currentNode->best_time + travel_time < nextNode->best_time) {

                nextNode->edge_in = currentEdge;
                nextNode->best_time = currentNode->best_time + travel_time; 

                // Queue the new wave element with newly approximated travel_time
                wavefront.push(waveElem(nextNode, currentEdge, currentElem.travel_time 
                        + travel_time));
            }
        }
        if (currentNode->intersection_id == destID) {return true; }
        
    } 
    
    return false;
}