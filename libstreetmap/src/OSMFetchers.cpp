/*
 * Functions used to fetch extra data from the level-1 api
 */

#include "OSMFetchers.h"
#include "OSMDatabaseAPI.h"
#include "map_db.h"
#include "helper_functions.h"

void load_osm_data() {
    
    for(unsigned id = 0; id < getNumberOfNodes(); id ++) {
        const OSMNode* node = getNodeByIndex(id);
        
        for(unsigned i=0; i < getTagCount(node); i++) {
            std::string key, value;
            std::tie(key, value) = getTagPair(node, i);

            if(value == "station") {
                MAP.OSM_data.subway_entrances.push_back( point2d_from_LatLon(node->coords()) );
            }

        }
    }
    
    for(unsigned id = 0; id < getNumberOfWays(); id ++) {
        const OSMWay* way = getWayByIndex(id);
        
        for(unsigned i=0; i < getTagCount(way); i++) {
            std::string key, value;
            std::tie(key, value) = getTagPair(way, i);
            
            if(key == "railway" && value == "subway") {
                for(unsigned j=0; j<way->ndrefs().size(); j++) {
                    const OSMNode* node = getNodeByIndex(way->ndrefs()[j]);
                    
                    //MAP.OSM_data.subway_path[i].push_back( point2d_from_LatLon(way->ndrefs()[j]) );
                }
            }
                                                       
        }
    }

}