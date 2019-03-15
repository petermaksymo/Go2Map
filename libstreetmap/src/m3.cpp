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

ezgl::point2d get_other_segment_point(int intersection_id, InfoStreetSegment & segment, StreetSegmentIndex segment_id);

#define NO_EDGE -1

// A node represent a single intersection of the map
class Node {
    private:
    int intersection_id; // ID of intersection the node contains
    int edge_in; // The route taken coming into the node
    double travel_time;
    std::vector<int> edge_out; // All street segment going out of the intersection  
    
    public:
    Node(int id, int edge, double time) {
        intersection_id = id;
        edge_in = edge;
        travel_time = time;
        
        // Insert every single connected street segment into outgoing_
        for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id); i++) {
            // Handling of single-way case is needed 
            edge_out.push_back(getIntersectionStreetSegment(i, intersection_id));
        }
        
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
            ? getStreetSegmentCurvePoint(0, segment_id)
            : getStreetSegmentCurvePoint(segment.curvePointCount-1, segment_id);
    }
    
    return point2d_from_LatLon(point_LL);
}


double compute_path_travel_time(const std::vector<unsigned>& path, 
                                const double right_turn_penalty, 
                                const double left_turn_penalty) {
    return 0.0;
    
}

std::vector<unsigned> find_path_between_intersections(
		  const unsigned intersect_id_start, 
                  const unsigned intersect_id_end,
                  const double right_turn_penalty, 
                  const double left_turn_penalty) {
    // Initialize the starting intersection
    Node* sourceNode = new Node(intersect_id_start, NO_EDGE, 0);  
    bfsPath(sourceNode, intersect_id_end);
    
}

bool bfsPath(Node* sourceNode, int destID) {
    // Initialize queue for BFS
    std::vector<Node* > wavefront;
    
    // Queue the source node 
    wavefront.push_back(sourceNode);
    

    
    // Do bfs while the wavefront is not empty
    while (!wavefront.empty()) {
        
    } 
    
    return false;
}