/*
 * Functions used to fetch extra data from the level-1 api
 */

#include "OSMFetchers.h"
#include "OSMDatabaseAPI.h"

void load_osm_data() {
    std::cout << "Number of nodes: " << getNumberOfNodes() << "\n"
              << "Number of ways: " << getNumberOfWays() << "\n"
              << "Number of relations " << getNumberOfRelations() <<"\n\n";
    
    int num_entrances = 0;
    for(unsigned id = 0; id < getNumberOfNodes(); id ++) {
        const OSMEntity* e = getNodeByIndex(id);
        for(unsigned j=0; j < getTagCount(e); j++) {
            std::string key, value;
            std::tie(key, value) = getTagPair(e, j);

            if(value == "subway_entrance") {
                //std::cout<< "found an entrance\n";
                num_entrances ++;
            }

        }
    }
    
    //std::cout << "found " << num_entrances << " entrances in total!!\n\n";
}