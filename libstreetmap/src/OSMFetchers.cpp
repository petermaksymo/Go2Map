/*
 * Functions used to fetch extra data from the level-1 api
 */

#include "OSMFetchers.h"
#include "OSMDatabaseAPI.h"
#include "map_db.h"
#include "helper_functions.h"


void load_osm_data() {
    //builds OSMID lookup hash tables and populates MAP with needed extra data
    go_through_OSM_nodes();
    go_through_OSM_ways();
    go_through_OSM_relations();
}


void go_through_OSM_nodes() {
    //should help with performance by reserving before hand
    MAP.OSM_data.node_by_OSMID.reserve(getNumberOfNodes());
    
    //flag needed for adding ttc stations
    bool found_ttc = false;
    
    //build a hash table of the nodes for querying and load node data at the same time
    for(unsigned id = 0; id < getNumberOfNodes(); id ++) {        
        const OSMNode* node = getNodeByIndex(id);
        MAP.OSM_data.node_by_OSMID.insert(std::make_pair(node->id(), node));
        
        for(unsigned i=0; i < getTagCount(node); i++) {
            std::string key, value;
            std::tie(key, value) = getTagPair(node, i);

            //hard-coded to add some more ttc stations because the data is old so 
            //they aren't listed in relations
            if(key == "operator" && value == "Toronto Transit Commission") {
                add_ttc_station(node, found_ttc);
            }
        }
    }
}


void go_through_OSM_ways() {
    //should help with performance by reserving before hand
    MAP.OSM_data.way_by_OSMID.reserve(getNumberOfWays());
    
    //build a hash table of the ways for querying and load way data at the same time
    for(unsigned id = 0; id < getNumberOfWays(); id ++) {        
        const OSMWay* way = getWayByIndex(id);
        MAP.OSM_data.way_by_OSMID.insert(std::make_pair(way->id(), way));
        
        for(unsigned i=0; i < getTagCount(way); i++) {
            std::string key, value;
            std::tie(key, value) = getTagPair(way, i);
            
            //find bike routes using all applicable tags
            if((key == "route" && value == "bicycle") ||
               (key == "highway" && value == "cycleway") ||
               (key == "bicycle_road" && value == "yes") ||
               (key == "bicycle" && value == "yes") ||
               (key == "cycleway") ) {
                
                //add a vector of points in that way to bike_routes vector
                MAP.OSM_data.bike_routes.push_back(point_vector_from_way(way));
            }
        }
    }
}


void go_through_OSM_relations() {
    for(unsigned id = 0; id < getNumberOfRelations(); id ++) {
        const OSMRelation* relation = getRelationByIndex(id);
        
        for(unsigned i=0; i < getTagCount(relation); i++) {
            std::string key, value;
            std::tie(key, value) = getTagPair(relation, i);
            
            if(key == "route" && value == "subway") {
                add_subway_route(relation);
            }
                                                       
        }
    }
}


void add_subway_route(const OSMRelation* relation) {
    //build the data in a placeholder, added to MAP at the end
    SubwayRouteData route;

    //go through each member
    for(unsigned member = 0; member < relation->members().size(); member ++) {
        //Add nodes (stations))
        if(relation->members()[member].tid.type() == TypedOSMID::Node) {
            const OSMNode* station = MAP.OSM_data.node_by_OSMID[relation->members()[member].tid];
            route.stations.push_back(point2d_from_LatLon( station->coords()) );
        } 
        
        //Add ways (tracks), this is a double vector to already have all points in a way for drawing
        //Can't just have every point for every way in 1 vector as it doesn't draw properly
        else if(relation->members()[member].tid.type() == TypedOSMID::Way) {
            const OSMWay* path = MAP.OSM_data.way_by_OSMID[relation->members()[member].tid];
            std::vector<ezgl::point2d> points;

            route.path.push_back(point_vector_from_way(path));
        }

    }
    //add the data
    MAP.OSM_data.subway_routes.push_back(route);
}


void add_ttc_station(const OSMNode* node, bool &found_ttc) {
    for(unsigned j=0; j < getTagCount(node); j++) {
        std::string key2, value2;
        std::tie(key2, value2) = getTagPair(node, j);

        if(key2 == "next_train") {
            if(!found_ttc) {
                SubwayRouteData placeholder;
                MAP.OSM_data.subway_routes.push_back(placeholder);
                found_ttc = true;
            }

            MAP.OSM_data.subway_routes[0].stations.push_back(point2d_from_LatLon(node->coords()) );
        }
    }
}


std::vector<ezgl::point2d> point_vector_from_way(const OSMWay* way) {
    std::vector<ezgl::point2d> points;

    for(unsigned node_in_path=0; node_in_path < way->ndrefs().size(); node_in_path++) {
        const OSMNode* node = MAP.OSM_data.node_by_OSMID[way->ndrefs()[node_in_path]]; 
        points.push_back( point2d_from_LatLon(node->coords()) );
    }

    return points;
}