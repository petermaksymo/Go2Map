/* 
 * Copyright 2018 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "m1.h"
#include "StreetsDatabaseAPI.h"

bool load_map(std::string map_path) {
    bool load_successful = loadStreetsDatabaseBIN(map_path);
   
    if(not load_successful) return false;
    
    //
    //Load your map related data structures here
    // The time constraint is 3000ms for load_map

    


    return load_successful;
}

void close_map() {
    //Clean-up your map related data structures here
    closeStreetDatabase();
    
}

std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id) {
    std::vector<unsigned> street_segments;
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id); i++ ) {
        street_segments.push_back(getIntersectionStreetSegment(i, intersection_id));
    }
    
    return street_segments;
}

std::vector<std::string> find_intersection_street_names(unsigned intersection_id) {
    std::vector<std::string> street_names;
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id); i++ ) {
        StreetIndex street_index = getInfoStreetSegment(getIntersectionStreetSegment(i, intersection_id)).streetID;
               
        street_names.push_back(getStreetName(street_index));
    }
    
    return street_names;
}

bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2) {
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id1); i++ ) {
        StreetSegmentIndex street_segment_index = getIntersectionStreetSegment(i, intersection_id1); 
        
        if(getInfoStreetSegment(street_segment_index).to == intersection_id2 
            || getInfoStreetSegment(street_segment_index).from == intersection_id2 ) 
                return true;
    }
    
    return false;
}

std::vector<unsigned> find_adjacent_intersections(unsigned intersection_id) {
    std::vector<unsigned> adjacent_intersections;
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id); i++ ) {
        StreetSegmentIndex street_segment_index = getIntersectionStreetSegment(i, intersection_id); 
        InfoStreetSegment s_segment = getInfoStreetSegment(street_segment_index);
        IntersectionIndex to_add = getNumIntersections() + 1;
         
        if(s_segment.oneWay) {
            if(s_segment.from == intersection_id)
                to_add = s_segment.to;
        } else
            to_add = s_segment.to == intersection_id ? s_segment.from : s_segment.to;
        
        bool unique = true;
        for(std::vector<unsigned>::iterator it = adjacent_intersections.begin(); it != adjacent_intersections.end(); it++) {
            if(*it == to_add) unique = false;
        }
        if(unique && to_add < getNumIntersections() + 1) adjacent_intersections.push_back(to_add);
       
    }
    
    return adjacent_intersections;
}

std::vector<unsigned> find_street_street_segments(unsigned street_id) {
    std::vector<unsigned> street_segments = {0, 1};
    
    return street_segments;
}

std::vector<unsigned> find_all_street_intersections(unsigned street_id) {
    std::vector<unsigned> street_intersections = {0, 1};
    
    return street_intersections;
}

std::vector<unsigned> find_intersection_ids_from_street_ids(unsigned street_id1, 
                                                              unsigned street_id2) {
    std::vector<unsigned> intersection_ids = {0, 1};
    
    return intersection_ids;
}

double find_distance_between_two_points(LatLon point1, LatLon point2) {
    return 0.0;
}

double find_street_segment_length(unsigned street_segment_id) {
    return 0.0;
}

double find_street_length(unsigned street_id) {
    return 0.0;
}

double find_street_segment_travel_time(unsigned street_segment_id) {
    return 0.0;
}

unsigned find_closest_point_of_interest(LatLon my_position) {
    return 0;
}

unsigned find_closest_intersection(LatLon my_position) {
    return 0;
}

std::vector<unsigned> find_street_ids_from_partial_street_name(std::string street_prefix) {
    std::vector<unsigned> street_ids = {0, 1};
    
    return street_ids;
}
