/*
 * Contains the code for m4, solving the travelling courier problem
 */

#include "m4.h"
#include "m3.h"
#include <iostream>

#define NOT_DELIVERY -1
enum stop_type {PICK_UP, DROP_OFF, DEPOT};

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

//simple legality checker, operates in O(N), N = route size
bool check_legal_simple(
        std::vector<RouteStop> &route, 
        std::vector<bool> &is_in_truck, 
        const std::vector<DeliveryInfo>& deliveries, 
        double capacity
);

std::vector<CourierSubpath> traveling_courier(
		const std::vector<DeliveryInfo>& deliveries,
	       	const std::vector<unsigned>& depots, 
		const float right_turn_penalty, 
		const float left_turn_penalty, 
		const float truck_capacity) {
    
    //initialize for legality checking
    std::vector<bool> is_in_truck(deliveries.size(), false);
    
    
    //Create a simple path (for this its 0-0, 1-1, 2-2... and first depot)
    std::vector<RouteStop> route;
    route.push_back(RouteStop(depots[0], NOT_DELIVERY, DEPOT, 0));
    
    int delivery_index = 0;
    for(auto &delivery : deliveries) {
        route.push_back(RouteStop(delivery.pickUp , delivery_index, PICK_UP, 0));
        route.push_back(RouteStop(delivery.dropOff, delivery_index, DROP_OFF, 0));
        
        delivery_index ++;
    }
    
    route.push_back(RouteStop(depots[0], NOT_DELIVERY, DEPOT, 0));
    
    
    //Convert simple path to one we can return:
    std::vector<CourierSubpath> route_complete, empty;
    
    //check legality
    if(not check_legal_simple(route, is_in_truck, deliveries, truck_capacity))
        return empty;
    
    
    for(auto stop = route.begin(); stop != route.end()-1; ++ stop) {
        CourierSubpath path;
        path.start_intersection = (*stop).intersection_id;
        path.end_intersection = (*(stop+1)).intersection_id;
        std::vector<unsigned> subpath = find_path_between_intersections(
            path.start_intersection,
            path.end_intersection,
            right_turn_penalty,
            left_turn_penalty
        );
        if(subpath.size() == 0) return empty; //no possible route
        path.subpath = subpath;
        std::vector<unsigned> pickUps;
        if((*stop).type == PICK_UP) pickUps.push_back( (*stop).delivery_index );
        path.pickUp_indices = pickUps;
        
        route_complete.push_back(path);
    }
    
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
            case DEPOT:
                break;
            default: 
                std::cout << "Error, invalid stop type in legality check\n";
                return false;    
        }
        
        if(current_weight > capacity) return false;
    }
    
    return true;
}