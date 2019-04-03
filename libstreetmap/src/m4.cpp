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
#include <thread>
#include <algorithm>
#include <chrono>
#include "helper_functions.h"
#include <omp.h>


#define NO_ROUTE std::numeric_limits<unsigned>::max()

enum stop_type {PICK_UP, DROP_OFF};

//for fast random number generator
static uint64_t mcg_state;
static uint64_t const multiplier = 6364136223846793005u;

#define TIME_LIMIT 40
#define NO_ROUTE std::numeric_limits<unsigned>::max()

struct RouteStop {
    //used for the route to accelerate mutations
    
    RouteStop()
        : intersection_id(0), 
          delivery_index(0), 
          type(PICK_UP) {};
    
    RouteStop(unsigned _intersection_id, int _delivery_index, stop_type _type)
        : intersection_id(_intersection_id), 
          delivery_index(_delivery_index), 
          type(_type){};
    
    //the intersection id
    unsigned intersection_id;
    
    //the index of the stop in the deliveries vector
    int delivery_index;
    
    //whether this stop is a pickUp
    stop_type type;
};

//fast random number generator and values needed for it, use unused attribute to suppress warnings
extern uint64_t mcg_state;
uint32_t pcg32_fast() __attribute__ ((hot));

//initialize fast random number generator
void pcg32_fast_init(uint64_t seed);

void multi_dest_dijkistra(
		  const unsigned intersect_id_start, 
                  const unsigned row_index,
                  std::vector<Node*> &intersection_nodes,
                  std::vector<unsigned> dests,
                  const float right_turn_penalty, 
                  const float left_turn_penalty
); 

void clear_intersection_nodes(std::vector<Node*> &intersection_nodes);

bool check_legal_simple(
        std::vector<RouteStop> &route, 
        std::vector<bool> &is_in_truck, 
        const std::vector<DeliveryInfo>& deliveries, 
        float capacity
);

void build_route(
        std::vector<RouteStop> &simple_route,
        std::vector<CourierSubpath> &complete_route,
        const float right_turn_penalty, 
        const float left_turn_penalty 
);


void add_closest_depots_to_route(
        std::vector<RouteStop> &simple_route,
        const std::vector<unsigned>& depots
);

bool validate_route(std::vector<RouteStop> &route, 
        double &time,
        std::vector<bool> &is_in_truck, 
        const std::vector<DeliveryInfo>& deliveries, 
        const float &capacity
) __attribute__ ((hot));

//returns time of route
double get_route_time(std::vector<RouteStop> &route);

std::pair<unsigned, unsigned> random_swap(std::vector<RouteStop> &route);

void two_opt_swap_annealing(std::vector<RouteStop> &route,
                         double run_time,
                         std::vector<bool> &is_in_truck, 
                         const std::vector<DeliveryInfo>& deliveries, 
                         float capacity,
                         int annealing);

void two_opt_swap_annealing_temp(std::vector<RouteStop> &route,
                         double &min_time,
                         std::vector<bool> &is_in_truck, 
                         const std::vector<DeliveryInfo>& deliveries, 
                         const float &capacity,
                         float &temp
)  __attribute__ ((hot, flatten));

std::pair<int, int> random_edge_swap(std::vector<RouteStop> &route) __attribute__ ((hot));

void reverse_vector(std::vector<RouteStop> &route, int &edge1, int &edge2) __attribute__ ((hot));

void find_greedy_path(const std::vector<unsigned> &destinations,
                      const std::vector<DeliveryInfo>& deliveries,
                      std::vector<RouteStop> &route,
                      const float truck_capacity);

//////////////////////////////////////////////////////////////////////////
//Start of functions
std::vector<CourierSubpath> traveling_courier(
		const std::vector<DeliveryInfo>& deliveries,
	       	const std::vector<unsigned>& depots, 
		const float right_turn_penalty, 
		const float left_turn_penalty, 
		const float truck_capacity) 
{
    auto startTime = std::chrono::high_resolution_clock::now();
    bool timeOut = false;

    //Clean and resize the 2D matrix to appropriate size
    MAP.courier.time_between_deliveries.clear();
    MAP.courier.time_between_deliveries.assign(deliveries.size() * 2  + depots.size(), 
    std::vector<unsigned>(deliveries.size() * 2, NO_ROUTE));
    
    // The destinations alternate between pickup and dropoff;
    // To access certain pickup: index * 2
    // To access certain dropoff: index * 2 + 1
    std::vector<unsigned> destinations;
    for (auto it = deliveries.begin(); it != deliveries.end(); ++it) {
        destinations.push_back(it->pickUp);
        destinations.push_back(it->dropOff);
    }
    
    std::vector<RouteStop> best_route;
    double best_time = std::numeric_limits<double>::max();
    
    //each thread needs its own mcg_state for random number generation
    #pragma omp threadprivate(mcg_state)
    #pragma omp parallel
    {    
        mcg_state = 0xcafef00dd15ea5e5u; // Must be odd, used for fast random 
        
        //initialize fast random number generator
        pcg32_fast_init(rand());
        
        //initiallize a node vector for each thread
        std::vector<Node*> intersection_nodes;
        intersection_nodes.resize(getNumIntersections());
        for(int i = 0; i < getNumIntersections(); i++) {
            intersection_nodes[i] = (new Node(i, NO_EDGE, 0));
        }
        
        //split the load of the for loop for each thread
        #pragma omp for
        for (unsigned i = 0; i < destinations.size(); ++i) {
            multi_dest_dijkistra(destinations[i], i, intersection_nodes, 
                    destinations, right_turn_penalty, left_turn_penalty);
        }
        
        
        #pragma omp for
        // Add depots to all pickup/dropoff location time to the vector
        for (unsigned i = 0; i < depots.size(); ++i) {
            multi_dest_dijkistra(depots[i], i + destinations.size(), intersection_nodes, 
                    destinations, right_turn_penalty, left_turn_penalty);
        }
        
        
        //delete the nodes now for each thread
        for(int i = 0; i < getNumIntersections(); i++) {
            delete intersection_nodes[i];
        }
        
        //wait for multi_dest_dijkstras to be done
        #pragma omp barrier

        
        //Start optimizations with simulated annealing
        std::vector<RouteStop> route;
        find_greedy_path(destinations, deliveries, route, truck_capacity);
        
        //initialize for legality checking
        std::vector<bool> is_in_truck(deliveries.size(), false);        
        
        //set min time
        double min_time = 0;
        bool initial_check = validate_route(route, min_time, is_in_truck, deliveries, truck_capacity);
        //if bad route, re-run greedy (greedy is currently bugged)
        while(!initial_check) {
            route.clear();
            min_time = 0;
            find_greedy_path(destinations, deliveries, route, truck_capacity);
            initial_check = validate_route(route, min_time, is_in_truck, deliveries, truck_capacity);
        }
        
        //store absolute best time/route for thread
        std::vector<RouteStop> best_route_to_now = route;
        double best_time_to_now = min_time;
        
        float temp = 10;

        double new_time;
        bool is_legal;
        
        int runs = 0, better = 0, best = 0, total = 0, legal = 0;
        // Loop over calling random swap until the time runs out
        while(!timeOut) {
            runs++;
            //two_opt_swap_annealing_temp(route, min_time, is_in_truck, deliveries, truck_capacity, temp);
            
            // Check if the algorithm has timed out
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto wallClock = std::chrono::duration_cast<std::chrono::duration<double>> (currentTime - startTime);

            timeOut = wallClock.count() > TIME_LIMIT;
            
            float x = (( TIME_LIMIT - wallClock.count()) - 1.0)/16.0;       
            //adjust annealing temp
            temp = exp(x) - 1;
            if(x<0) temp = 0;
                     
            
            // Break and swap two edges randomly
            std::pair<int, int> indexes = random_edge_swap(route);

            // Try to get new time (in if statement)
            is_legal = validate_route(route, new_time, is_in_truck, deliveries, truck_capacity);
            if(is_legal) legal ++;
            
            // If a route can be found, OR is annealing, it improves travel time and it passes the legal check,
            // then keep the the new route, otherwise reverse the changes
            if(is_legal && (new_time < min_time || ((1.0/(float)pcg32_fast() < exp(-1*(new_time - min_time)/temp))) )// for simulated annealing
            ) {
                if(new_time < min_time) better++;
                total++;
                min_time = new_time;
                
                if(min_time < best_time_to_now) {
                   best ++;
                   best_route_to_now = route;
                   best_time_to_now = min_time; 
                }
            } else {
                reverse_vector(route, indexes.first, indexes.second);
            }
                        
        }
                
        //each thread takes a turn comparing its result to best overall
        #pragma omp critical
        {
            /*std::cout << "runs: " << runs << " best time: "<< best_time_to_now << "  thread#: " 
                    << omp_get_thread_num() << "  best swaps: " << best << "  better swaps: " << better 
                    << "  total swaps : " << total << "  legal options: " << legal << "\n";*/
            add_closest_depots_to_route(best_route_to_now, depots);
            best_time_to_now = get_route_time(route);
            if(best_time_to_now < best_time) {
                best_route = best_route_to_now;
                best_time = best_time_to_now;
            }
        }
        
    }   

//    add_closest_depots_to_route(best_route, depots, right_turn_penalty, left_turn_penalty);
    
    //Convert simple path to one we can return:
    std::vector<CourierSubpath> route_complete;
    build_route(best_route, route_complete, right_turn_penalty, left_turn_penalty);    
    
    return route_complete;
}


//Fast random number generator, the next 2 functions are from:
//https://en.wikipedia.org/wiki/Permuted_congruential_generator#Example_code
uint32_t pcg32_fast() {
    uint64_t x = mcg_state;
    unsigned count = (unsigned)(x >> 61); // 61 = 64 - 3
    
    mcg_state = x * multiplier;
    x ^= x >> 22;
    return (uint32_t)(x >> (22 + count)); // 22 = 32 - 3 - 7
}


void pcg32_fast_init(uint64_t seed) {
    mcg_state = 2*seed +1;
    (void)pcg32_fast();
}


//quickly checks legality operates in O(n), currently deprecated 
bool check_legal_simple(
        std::vector<RouteStop> &route, 
        std::vector<bool> &is_in_truck, 
        const std::vector<DeliveryInfo>& deliveries, 
        float capacity
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
                  std::vector<Node*> &intersection_nodes,
                  std::vector<unsigned> dests,
                  const float right_turn_penalty, 
                  const float left_turn_penalty) {
    unsigned num_found = 0;
    //Node& sourceNode = MAP.intersection_node[intersect_id_start];
    // Initialize queue for BFS
    std::priority_queue <waveElem, std::vector<waveElem>, comparator> wavefront; 
   
    // Queue the source node 
    waveElem sourceElem = waveElem(intersection_nodes[intersect_id_start],
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
                    ? intersection_nodes[edgeInfo.to]
                    : intersection_nodes[edgeInfo.from];
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
        if (num_found == dests.size()) { 
            clear_intersection_nodes(intersection_nodes);
            return;
        }
        
        // Remove a destination from the dests vector once the shortest route to it is found
        int i = 0;
        for (auto it = dests.begin(); it != dests.end(); ++it, ++i) {            
            if ((unsigned)currentNode->intersection_id == *it) {
                if(MAP.courier.time_between_deliveries [row_index][i] == NO_ROUTE) num_found ++;
                
                if(intersect_id_start == (unsigned)currentNode->intersection_id) {
                    MAP.courier.time_between_deliveries [row_index][i] = 0;
                } else {
                    MAP.courier.time_between_deliveries [row_index][i] = currentNode->best_time;
                }
            }
        }
        
    } 
    
    clear_intersection_nodes(intersection_nodes);
    //std::cout << "No valid routes are found" << std::endl;

}


void clear_intersection_nodes(std::vector<Node*> &intersection_nodes) {
    
    for(int i = 0; i < getNumIntersections(); i++) {
        intersection_nodes[i]->best_time = 0;
        intersection_nodes[i]->edge_in = NO_EDGE;
    }        
}

void add_closest_depots_to_route(
        std::vector<RouteStop> &simple_route,
        const std::vector<unsigned>& depots
) {
    double start_min = -1;
    double end_min = -1;
    unsigned int start_it = 0;
    unsigned int end_it = 0;
    
    // Loop over the depots, calling the m3 functions to calculate the time to each depot
    for(unsigned i = 0; i < depots.size(); ++i) {
        double start_time = MAP.courier.time_between_deliveries[i + MAP.courier.time_between_deliveries[0].size()][simple_route[0].delivery_index * 2];
        //auto start_route = find_path_between_intersections(simple_route[0].intersection_id, *it, right_turn_penalty, left_turn_penalty);
        if (start_time < std::numeric_limits<unsigned>::max()) {
            //double start_time = compute_path_travel_time(start_route, right_turn_penalty, left_turn_penalty);
            if(start_min == -1 || start_time < start_min) {
                start_it = depots[i];
                start_min = start_time;
            }
        }
        
        double end_time = MAP.courier.time_between_deliveries[i + MAP.courier.time_between_deliveries[0].size()][simple_route[simple_route.size() - 1].delivery_index * 2];
        //auto end_route = find_path_between_intersections(simple_route[simple_route.size() - 1].intersection_id, *it, right_turn_penalty, left_turn_penalty);
        if(end_time < std::numeric_limits<unsigned>::max()) {
            //double end_time = compute_path_travel_time(end_route, right_turn_penalty, left_turn_penalty);
            
            if(end_min == -1 || end_time < end_min) {
                end_it = depots[i];
                end_min = end_time;
            }
        }
    }
 
    simple_route.insert(simple_route.begin(), RouteStop(start_it, -1, DROP_OFF));
    simple_route.push_back(RouteStop(end_it, -1, DROP_OFF));
}

//returns time of route and has legal flag, can set to false if non-reachable route, currently deprecated
double get_route_time(std::vector<RouteStop> &route){
    double time = 0;
    
    for(auto stop = route.begin(); stop != route.end()-1; ++stop) {
        int i = (*stop).type == PICK_UP ? (*stop).delivery_index*2 : (*stop).delivery_index*2+1;
        int j = (*(stop+1)).type == PICK_UP ? (*(stop+1)).delivery_index*2 : (*(stop+1)).delivery_index*2+1;
        
        time += (double)MAP.courier.time_between_deliveries[i][j];
    }
    
    return time;
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
        path.subpath = subpath;
        std::vector<unsigned> pickUps;
        if((*stop).type == PICK_UP) pickUps.push_back( (*stop).delivery_index );
        path.pickUp_indices = pickUps;
        
        complete_route.push_back(path);
    }
}

//returns time of route and has legal flag, can set to false if non-reachable route
bool validate_route(std::vector<RouteStop> &route, 
        double &time,
        std::vector<bool> &is_in_truck, 
        const std::vector<DeliveryInfo>& deliveries, 
        const float &capacity
){
    //set all to false (should be able to delete once fully integrated)
    is_in_truck.assign(is_in_truck.size(), false);
    
    float current_weight = 0;
    time = 0;
    
    for(auto stop = route.begin(); stop != route.end()-1; ++stop) {
        int i = (*stop).type == PICK_UP ? (*stop).delivery_index*2 : (*stop).delivery_index*2+1;
        
        //check legality
        switch((*stop).type) {
            case PICK_UP:
                is_in_truck[(*stop).delivery_index] = true; 
                current_weight += deliveries[(*stop).delivery_index].itemWeight;
                break;
            case DROP_OFF:
                current_weight -= deliveries[(*stop).delivery_index].itemWeight;
                if(not is_in_truck[(*stop).delivery_index])
                    return false;
                break;
            default: 
                std::cout << "Error, invalid stop type in legality check\n";
                return false;    
        }

        if(current_weight > capacity) return false;

        //now add time
        int j = (*(stop+1)).type == PICK_UP ? (*(stop+1)).delivery_index*2 : (*(stop+1)).delivery_index*2+1;

        if(MAP.courier.time_between_deliveries[i][j] == NO_ROUTE) return false;

        time += (float)MAP.courier.time_between_deliveries[i][j];
        
        i = j;
    }
    
    return true;
}

// Reverses the part of a vector between two positions
void reverse_vector(std::vector<RouteStop> &route, int &edge1, int &edge2) {
    std::reverse(route.begin() + edge1, route.begin() + edge1 + edge2);
    return;
}

// Randomly selects two edges in the array, ie two positions
// and reverses the part of the vector between those positions
std::pair<int, int> random_edge_swap(std::vector<RouteStop> &route) {
    int edge1 = pcg32_fast() % (int)route.size();
    int edge2 = pcg32_fast() % (route.size() - edge1);
    
    reverse_vector(route, edge1, edge2);
    
    return std::make_pair(edge1, edge2);
}


// Continously swaps two edges, keeping the shortest time
// until it runs out of run_counts
void two_opt_swap_annealing_temp(std::vector<RouteStop> &route,
                         double &min_time,
                         std::vector<bool> &is_in_truck, 
                         const std::vector<DeliveryInfo>& deliveries, 
                         const float &capacity,
                         float &temp
) {
    
    // Break and swap two edges randomly
    std::pair<int, int> indexes = random_edge_swap(route);

    // Try to get new time (in if statement)
    double new_time;
    bool is_legal = validate_route(route, new_time, is_in_truck, deliveries, capacity);

    // If a route can be found, OR is annealing, it improves travel time and it passes the legal check,
    // then keep the the new route, otherwise reverse the changes
    if(is_legal && (new_time < min_time || ((1.0/(float)pcg32_fast() < exp(-1*(new_time - min_time)/temp))) )// for simulated annealing
    ) {
        min_time = new_time;
    } else {
        reverse_vector(route, indexes.first, indexes.second);
    }
}

// Continously swaps two edges, keeping the shortest time
// until it runs out of run_counts
void two_opt_swap_annealing(std::vector<RouteStop> &route,
                         double run_time,
                         std::vector<bool> &is_in_truck, 
                         const std::vector<DeliveryInfo>& deliveries, 
                         float capacity,
                         int annealing
) {
    
    bool is_legal = true;
    double min_time = get_route_time(route);
        
    // For run_time of algorithm
    auto startTime = std::chrono::high_resolution_clock::now();
    bool timeOut = false;
    
    int temp = annealing;
    
    // Loop over calling random swap until the time runs out
    while(!timeOut) {
        
        // Break and swap two edges randomly
        std::pair<int, int> indexes = random_edge_swap(route);
 
        // Try to get new time
        double new_time = get_route_time(route);
        
        // If a route can be found, OR is annealing, it improves travel time and it passes the legal check,
        // then keep the the new route, otherwise reverse the changes
        if(is_legal &&
          (new_time < min_time || (temp > 0 && pcg32_fast() % temp == 0)) && // for simulated annealing
          check_legal_simple(route, is_in_truck, deliveries, capacity)) {
            min_time = new_time;
        } else {
            reverse_vector(route, indexes.first, indexes.second);
            is_legal = true;
        }
        
        // Check if the algorithm has timed out
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto wallClock = std::chrono::duration_cast<std::chrono::duration<double>> (currentTime - startTime);

        
        timeOut = wallClock.count() >= run_time;
        temp = (run_time) / (run_time - wallClock.count()) + 1;
    }
}




// There are three kinds of index: delivery index, destination index, and intersection id
void find_greedy_path(const std::vector<unsigned> &destinations,
                      const std::vector<DeliveryInfo>& deliveries,
                      std::vector<RouteStop> &route,
                      const float truck_capacity) {
    
    // Randomly assign a pick up point as a starting point
    int current = (pcg32_fast() % (deliveries.size()-1)) * 2;
    if(deliveries.size() < 5) current = 0;
    
    int next = -1;
    double current_capacity_used = 0;
    std::vector<unsigned> current_item_carried;
    int item_to_deliver = deliveries.size();
    std::vector<bool> visited;
    visited.resize(destinations.size(), false);
    
    // Add the first pickup to current truck
    current_item_carried.push_back(current + 1);
    current_capacity_used += deliveries[current / 2].itemWeight;
    route.push_back(RouteStop(destinations[current], current / 2, PICK_UP));
    
    // Continue finding path until all items are delivered
    while (item_to_deliver != 0) {
        // <destination_index, time> for closest_pickup and closet_dropoff
        std::pair<int, int> closest_pickup, closest_dropoff; 
        
        // Initialize the pairs
        closest_pickup.first = -1;
        closest_dropoff.first = -1;
        closest_pickup.second = std::numeric_limits<int>::max();
        closest_dropoff.second = std::numeric_limits<int>::max();
        
        // Flag the visited node so it won't be visited again
        visited[current] = true;
        
        // Go through the time table to find nearest pickup and dropoff
        for (unsigned i = 0; i < MAP.courier.time_between_deliveries[0].size(); ++i) {
            int time = MAP.courier.time_between_deliveries[current][i];
            //if (current == i) std::cout << time << std::endl;
            //std::cout << i << " " << time << std::endl;
            // Different cases for pickup and dropoff
            if ((i % 2 == 0) && (time >= 0) && (time < closest_pickup.second) && !visited[i]) {
                closest_pickup.first = i;
                closest_pickup.second = time;
            } else if ((i % 2 == 1) && (time >= 0) && (time < closest_dropoff.second) && !visited[i]) {
                // Check if the item i is currently carried on the truck
                auto it = std::find(current_item_carried.begin(), current_item_carried.end(), i);
                if (current_item_carried.empty()) {
                    closest_dropoff.first = -1;
                    closest_dropoff.second = std::numeric_limits<int>::max();
                }
                else if (it != current_item_carried.end()) {
                    closest_dropoff.first = i;
                    closest_dropoff.second = time;
                } else if ((it == current_item_carried.end()) && (*it == i)) {
                    closest_dropoff.first = i;
                    closest_dropoff.second = time;
                }
            }
        }
        // Now determine the next destination
        // If truck cannot fit next nearest pickup or if dropoff is closer than pickup then go the nearest dropoff
        // Otherwise go the nearest pickup
        if (closest_pickup.first == -1 || current_capacity_used + deliveries[closest_pickup.first / 2].itemWeight > truck_capacity || 
            closest_dropoff.second < closest_pickup.second) {
            next = closest_dropoff.first;
            //std::remove(current_item_carried.begin(), current_item_carried.end(), closest_dropoff.first); // may be broken
            for (auto it = current_item_carried.begin(); it != current_item_carried.end(); ++it) {
                if ((int)*it == closest_dropoff.first) {
                    current_item_carried.erase(it);
                    break;
                }
            }
            current_capacity_used -= deliveries[closest_dropoff.first / 2].itemWeight;
            item_to_deliver -= 1;
            route.push_back(RouteStop(deliveries[closest_dropoff.first / 2].dropOff, closest_dropoff.first / 2, DROP_OFF));
        } else { 
            next = closest_pickup.first;
            current_item_carried.push_back(closest_pickup.first + 1); // Push the corresponding dropoff position to the vector
            current_capacity_used += deliveries[closest_pickup.first / 2].itemWeight;
            route.push_back(RouteStop(deliveries[closest_pickup.first / 2].pickUp, closest_pickup.first / 2, PICK_UP));
        }
        current = next;
    }
}

