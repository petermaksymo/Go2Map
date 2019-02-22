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
#include <iostream>
#include <string>

#include "map_db.h"
#include "m1.h"
#include "m2.h"
#include "KDTree.h"

//Program exit codes
constexpr int SUCCESS_EXIT_CODE = 0;        //Everyting went OK
constexpr int ERROR_EXIT_CODE = 1;          //An error occured
constexpr int BAD_ARGUMENTS_EXIT_CODE = 2;  //Invalid command-line usage

//The default map to load if none is specified
std::string default_map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";

int main(int argc, char** argv) {

    std::string map_path;
    if(argc == 1) {
        //Use a default map
        map_path = default_map_path;
    } else if (argc == 2) {
        //Get the map from the command line
        map_path = argv[1];
    } else {
        //Invalid arguments
        std::cerr << "Usage: " << argv[0] << " [map_file_path]\n";
        std::cerr << "  If no map_file_path is provided a default map is loaded.\n";
        return BAD_ARGUMENTS_EXIT_CODE;
    }

    //Load the map and related data structures
    bool load_success = load_map(map_path);
    if(!load_success) {
        std::cerr << "Failed to load map '" << map_path << "'\n";
        return ERROR_EXIT_CODE;
    }
    
    std::cout << "Successfully loaded map '" << map_path << "'\n";

    // Testing KD2Tree
    std::vector<std::pair<double, double>> test_vector_make;
    test_vector_make.push_back(std::make_pair(1,1));
    test_vector_make.push_back(std::make_pair(2,1));
    test_vector_make.push_back(std::make_pair(4,1));
    test_vector_make.push_back(std::make_pair(2,2));
    test_vector_make.push_back(std::make_pair(1,4));
    test_vector_make.push_back(std::make_pair(2,3));

    KD2Tree* k2tree = new KD2Tree(test_vector_make, 0);
    
    std::cout << "\nTree after initial make at zoom level 0 (showing zoom 1):" << std::endl;
    k2tree->visualize_tree(k2tree->root, 0, 1);
    
//    k2tree->insert_pair(k2tree->root, std::make_pair(3,1), 0);
//    k2tree->insert_pair(k2tree->root, std::make_pair(5,1), 0);
    
    std::vector<std::pair<double, double>> test_vector_bulk_insert;
    test_vector_bulk_insert.push_back(std::make_pair(3,1));
    test_vector_bulk_insert.push_back(std::make_pair(5,1));
    test_vector_bulk_insert.push_back(std::make_pair(6,1));
    test_vector_bulk_insert.push_back(std::make_pair(0,0));
    test_vector_bulk_insert.push_back(std::make_pair(10,10));
    
    k2tree->insert_bulk(test_vector_bulk_insert.begin(), // begin
                        test_vector_bulk_insert.end(), // end
                        k2tree->root, // root
                        0, // depth of insert
                        test_vector_bulk_insert.size(),
                        1);
    std::cout << "\nTree after bulk insert at zoom level 1 (showing ONLY zoom 0):" << std::endl;
    k2tree->visualize_tree(k2tree->root, 0, 0);
    
    std::cout << "\nTree after bulk insert at zoom level 1  (now showing zoom 1):" << std::endl;
    k2tree->visualize_tree(k2tree->root, 0, 1);
    
    std::vector<std::pair<double, double>> test_vector_range_results;
    
    k2tree->range_query(k2tree->root, 0, std::make_pair(0,2), std::make_pair(0,2), test_vector_range_results, 0);
    
    std::vector<std::pair<double, double>>::iterator it = test_vector_range_results.begin();
    
    std::cout << "\nQuery Results x[0,2] y[0,2] Zoom Level 0: " << std::endl;
    
    while(it != test_vector_range_results.end()) {
        std::cout << "(" << it->first << "," << it->second << ")" << std::endl;
        it++;
    }
    
    std::vector<std::pair<double, double>> test_vector_range_results2;
    
    k2tree->range_query(k2tree->root, 0, std::make_pair(0,2), std::make_pair(0,2), test_vector_range_results2, 1);
    
    std::vector<std::pair<double, double>>::iterator it2 = test_vector_range_results2.begin();
    
    std::cout << "\nQuery Results x[0,2] y[0,2] Zoom Level 1: " << std::endl;
    
    while(it2 != test_vector_range_results2.end()) {
        std::cout << "(" << it2->first << "," << it2->second << ")" << std::endl;
        it2++;
    }
    
    delete k2tree;
    
    //You can now do something with the map data
    draw_map();

    //Clean-up the map data and related data structures
    std::cout << "Closing map\n";
    close_map(); 

    return SUCCESS_EXIT_CODE;
}
