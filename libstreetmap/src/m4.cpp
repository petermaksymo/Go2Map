/*
 * Contains the code for m4, solving the travelling courier problem
 */

#include "m4.h"
#include "m3.h"
#include "constants.hpp"
#include "map_db.h"
#include <vector>
#include <iostream>
#include <bits/stdc++.h>

enum stop_type {PICK_UP, DROP_OFF};

struct RouteStop {
    //used for the route to accelerate mutations
    
    RouteStop(unsigned _intersection_id, int _delivery_index, stop_type _type, double _time_to_next)
        : intersection_id(_intersection_id), 
          delivery_index(_delivery_index), 
          type(_type),
          time_to_next(_time_to_next) {} 
    
    //the intersection id
    unsigned intersection_id;
    
    //the index of the stop in the deliveries vector
    int delivery_index;
    
    //whether this stop is a pickUp
    stop_type type;
    
    //time to the next node
    double time_to_next;
};

void multi_dest_dijkistra(
		  const unsigned intersect_id_start, 
                  const unsigned row_index,
                  std::vector<unsigned> dests,
                  const double right_turn_penalty, 
                  const double left_turn_penalty
); 

//simple legality checker, operates in O(N), N = route size
bool check_legal_simple(
        std::vector<RouteStop> &route, 
        std::vector<bool> &is_in_truck, 
        const std::vector<DeliveryInfo>& deliveries, 
        double capacity
);

void add_depots_to_route(
        std::vector<RouteStop> &simple_route,
        const std::vector<unsigned>& depots
        
);

void build_route(
        std::vector<RouteStop> &simple_route,
        std::vector<CourierSubpath> &complete_route,
        const float right_turn_penalty, 
        const float left_turn_penalty 
);


void add_closest_depots_to_route(
        std::vector<RouteStop> &simple_route,
        const std::vector<unsigned>& depots,
        const double right_turn_penalty, 
        const double left_turn_penalty
);


//////////////////////////////////////////////////////////////////////////
//Start of functions
std::vector<CourierSubpath> traveling_courier(
		const std::vector<DeliveryInfo>& deliveries,
	       	const std::vector<unsigned>& depots, 
		const float right_turn_penalty, 
		const float left_turn_penalty, 
		const float truck_capacity) {
    
    //Clean and resize the 2D matrix to appropriate size
    MAP.courier.time_between_deliveries.clear();
    MAP.courier.time_between_deliveries.resize(deliveries.size() * 2, std::vector<unsigned>(deliveries.size() * 2));
    
    // The destinations alternate between pickup and dropoff;
    // To access certain pickup: index * 2
    // To access certain dropoff: index * 2 + 1
    std::vector<unsigned> destinations;
    for (auto it = deliveries.begin(); it != deliveries.end(); ++it) {
        destinations.push_back(it->pickUp);
        destinations.push_back(it->dropOff);
    }
    
    int i = 0;
    for (auto it = deliveries.begin(); it != deliveries.end(); ++it, ++i) {
        multi_dest_dijkistra(destinations[i], i, destinations, right_turn_penalty, left_turn_penalty);
    }
    
    //initialize for legality checking
    std::vector<bool> is_in_truck(deliveries.size(), false);
  
    //Create a simple path (for this its 0-0, 1-1, 2-2... and first depot)
    std::vector<RouteStop> route;
    
    int delivery_index = 0;
    for(auto &delivery : deliveries) {
        route.push_back(RouteStop(delivery.pickUp , delivery_index, PICK_UP, 0));
        route.push_back(RouteStop(delivery.dropOff, delivery_index, DROP_OFF, 0));
        
        delivery_index ++;
    }
    
    //check legality
    if(not check_legal_simple(route, is_in_truck, deliveries, truck_capacity)){
        std::vector<CourierSubpath> empty;
        return empty;
    }
       
    add_depots_to_route(route, depots);
        
    //Convert simple path to one we can return:
    std::vector<CourierSubpath> route_complete;
    build_route(route, route_complete, right_turn_penalty, left_turn_penalty);    
    
    return route_complete;
}


//quickly checks legality operates in O(n)
bool check_legal_simple(
        std::vector<RouteStop> &route, 
        std::vector<bool> &is_in_truck, 
        const std::vector<DeliveryInfo>& deliveries, 
        double capacity
) {
    //set all to false (should be able to delete once fully integrated)
    is_in_truck.assign(is_in_truck.size(), false);
    
    double current_weight = 0;
    
    for(auto &stop : route) {
        switch(stop.type) {
            case PICK_UP:
                is_in_truck[stop.delivery_index] = true; 
                current_weight += deliveries[stop.delivery_index].itemWeight;
                break;
            case DROP_OFF:
                current_weight -= deliveries[stop.delivery_index].itemWeight;
                if(not is_in_truck[stop.delivery_index])
                    return false;
                break;
            default: 
                std::cout << "Error, invalid stop type in legality check\n";
                return false;    
        }
        
        if(current_weight > capacity) return false;
    }
    
    return true;
}


// It might be necessary to resize the MAP.courier.time_between_deliveries before calling this function
void multi_dest_dijkistra(
		  const unsigned intersect_id_start, 
                  const unsigned row_index,
                  std::vector<unsigned> dests,
                  const double right_turn_penalty, 
                  const double left_turn_penalty) {
    int dest_count = 0;
    //Node& sourceNode = MAP.intersection_node[intersect_id_start];
    // Initialize queue for BFS
    std::priority_queue <waveElem, std::vector<waveElem>, comparator> wavefront; 
   
    // Queue the source node 
    waveElem sourceElem = waveElem(MAP.intersection_node[intersect_id_start],
    NO_EDGE, 0.0); 
    
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
                        + travel_time + turn_penalty));
                }
            }
        }
        // Return if all the destinations are covered in the search
        if (dest_count == dests.size()) { 
            clear_intersection_node();
            return;
        }
        
        // Remove a destination from the dests vector once the shortest route to it is found
        int i = 0;
        for (auto it = dests.begin(); it != dests.end(); ++it, ++i) {
            if (currentNode->intersection_id == *it) {
                MAP.courier.time_between_deliveries [row_index][i] = currentNode->best_time;
                dest_count += 1; 
            }
          
        }
        
    } 
    
    clear_intersection_node();
    //std::cout << "No valid routes are found" << std::endl;
}


void add_depots_to_route(
        std::vector<RouteStop> &simple_route,
        const std::vector<unsigned>& depots
        
) {
    simple_route.insert(simple_route.begin(), RouteStop(depots[0], -1, DROP_OFF, 0));
    simple_route.push_back(RouteStop(depots[0], -1, DROP_OFF, 0));
}

void add_closest_depots_to_route(
        std::vector<RouteStop> &simple_route,
        const std::vector<unsigned>& depots,
        const double right_turn_penalty, 
        const double left_turn_penalty
) {
    double start_min = -1;
    double end_min = -1;
    unsigned int start_it = 0;
    unsigned int end_it = 0;

    // Loop over the depots, calling the m3 functions to calculate the time to each depot
    for(auto it = depots.begin(); it != depots.end(); it++) {
        double start_time = compute_path_travel_time(find_path_between_intersections(simple_route[0].intersection_id, *it, right_turn_penalty, left_turn_penalty), right_turn_penalty, left_turn_penalty);
        double end_time = compute_path_travel_time(find_path_between_intersections(simple_route[simple_route.size() - 1].intersection_id, *it, right_turn_penalty, left_turn_penalty), right_turn_penalty, left_turn_penalty);
        
        if(start_min == -1 || start_time < start_min) {
            start_it = *it;
            start_min = start_time;
        }
        
        if(end_min == -1 || end_time < end_min) {
            end_it = *it;
            end_min = end_time;
        }
    }
 
    simple_route.insert(simple_route.begin(), RouteStop(start_it, -1, DROP_OFF, 0));
    simple_route.push_back(RouteStop(end_it, -1, DROP_OFF, 0));
}

    
void build_route(
        std::vector<RouteStop> &simple_route,
        std::vector<CourierSubpath> &complete_route,
        const float right_turn_penalty, 
        const float left_turn_penalty
        
) {
    for(auto stop = simple_route.begin(); stop != simple_route.end()-1; ++ stop) {
        CourierSubpath path;
        path.start_intersection = (*stop).intersection_id;
        path.end_intersection = (*(stop+1)).intersection_id;
        std::vector<unsigned> subpath = find_path_between_intersections(
            path.start_intersection,
            path.end_intersection,
            right_turn_penalty,
            left_turn_penalty
        );
        if(subpath.size() == 0) {
            //no possible route
            complete_route.clear();
            return;
        }
        path.subpath = subpath;
        std::vector<unsigned> pickUps;
        if((*stop).type == PICK_UP) pickUps.push_back( (*stop).delivery_index );
        path.pickUp_indices = pickUps;
        
        complete_route.push_back(path);
    }
}