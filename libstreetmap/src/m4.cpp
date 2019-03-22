/*
 * Contains the code for m4, solving the travelling courier problem
 */

#include "m4.h"
#include "m3.h"

std::vector<CourierSubpath> traveling_courier(
		const std::vector<DeliveryInfo>& deliveries,
	       	const std::vector<unsigned>& depots, 
		const float right_turn_penalty, 
		const float left_turn_penalty, 
		const float truck_capacity) {
    std::vector<CourierSubpath> route, empty;
    
    std::vector<unsigned int> depot = {};
    
    //go from depot 0 to pick up 0
    CourierSubpath path_start;
    path_start.start_intersection = depots[0];
    path_start.end_intersection = deliveries[0].pickUp;
    std::vector<unsigned> subpath = find_path_between_intersections(depots[0],
            deliveries[0].pickUp,
            right_turn_penalty,
            left_turn_penalty
    );
    if(subpath.size() == 0) return empty;
    path_start.subpath = subpath;
    subpath.clear();
    path_start.pickUp_indices = depot;
    route.push_back(path_start);
    
    //pick up package and drop it off, then go to next pick up (0-0, 1-1...)
    for(unsigned i = 0; i < deliveries.size(); ++i) {
        CourierSubpath path_deliver;
        std::vector<unsigned int> pick_up = {i};
        //path from pick-up to drop-off
        path_deliver.start_intersection = deliveries[i].pickUp;
        path_deliver.end_intersection = deliveries[i].dropOff;
        subpath = find_path_between_intersections(deliveries[i].pickUp,
            deliveries[i].dropOff,
            right_turn_penalty,
            left_turn_penalty
        );
        if(subpath.size() == 0 || deliveries[i].itemWeight > truck_capacity) return empty;
        path_deliver.subpath = subpath;
        subpath.clear();
        path_deliver.pickUp_indices = pick_up;
        route.push_back(path_deliver);
        
        if(i < deliveries.size()-1) {
            CourierSubpath path_pickup;
            std::vector<unsigned int> drop_off = {};
            //path from drop-off to next pick-up
            path_pickup.start_intersection = deliveries[i].dropOff;
            path_pickup.end_intersection = deliveries[i+1].pickUp;
            subpath = find_path_between_intersections(deliveries[i].dropOff,
                deliveries[i+1].pickUp,
                right_turn_penalty,
                left_turn_penalty
            );
            if(subpath.size() == 0) return empty;
            path_pickup.subpath = subpath;
            subpath.clear();
            path_pickup.pickUp_indices = drop_off;
            route.push_back(path_pickup);
        }
    }
    
    //go from last drop off to depot 0
    CourierSubpath path_end;
    std::vector<unsigned int> empty_i = {};
    path_end.start_intersection = deliveries[deliveries.size()-1].dropOff;
    path_end.end_intersection = depots[0];
    subpath = find_path_between_intersections(deliveries[deliveries.size()-1].dropOff,
            depots[0],
            right_turn_penalty,
            left_turn_penalty
    );
    if(subpath.size() == 0) return empty;
    path_end.subpath = subpath;
    path_end.pickUp_indices = empty_i;
    route.push_back(path_end);
    
    return route;
    
}