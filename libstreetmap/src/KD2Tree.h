/* 
 * File:   KDTree.h
 *
 * KD Tree is a custom implementation of a classic KD Tree where k=2 with the added
 * feature of a zoom flag. The zoom flag allows queries to only check points
 * at certain zoom levels. It is based off crvs/KDTree on Github.
 * 
 */

#pragma once //protects against multiple inclusions of this header file

#include <vector>
#include <algorithm>
#include <map>

class KD2Node {
    public:
        std::pair<double, double> point; // [x, y]
        KD2Node* left; // less than
        KD2Node* right; // greater than or equal
        int zoom_level;
        unsigned int data_id;
        
        
        KD2Node();
        KD2Node(const std::pair<double, double> &pt, unsigned int data_id_, int zoom_level_);
        KD2Node(const std::pair<double, double> &pt, unsigned int data_id_, KD2Node* &left_, KD2Node* &right_, int zoom_level_);
        ~KD2Node();
};

class KD2Tree {
    public:
        KD2Node* root;
        
        KD2Tree();
        KD2Tree(std::vector<std::pair<std::pair<double, double>, unsigned int>> pts, const int &zoom_level);
        ~KD2Tree();
        
        KD2Node* make_tree(std::vector<std::pair<std::pair<double, double>, unsigned int>>::iterator, // begin
                          std::vector<std::pair<std::pair<double, double>, unsigned int>>::iterator, // end
                          const std::size_t &, // depth
                          const std::size_t &, // size of passed vector
                          const int &zoom_level);
        
        void visualize_tree(KD2Node* ptr, const std::size_t depth, const int &zoom_level);
        
        void insert_pair(KD2Node*, const std::pair<std::pair<double, double>, unsigned int> &, const std::size_t &, const int &zoom_level);
        
        void insert_bulk(std::vector<std::pair<std::pair<double, double>, unsigned int>>::iterator, // begin
                         std::vector<std::pair<std::pair<double, double>, unsigned int>>::iterator, // end
                         KD2Node* &, // root
                         const std::size_t &, // depth of insert
                         const std::size_t &, // size of passed vector
                         const int &zoom_level); // zoom level flag
        
        void range_query(KD2Node*, // root
                         const std::size_t &depth, // depth of query
                         const std::pair<double, double> &, // x-range (smaller, greater)
                         const std::pair<double, double> &, // y-range (smaller, greater)
                         std::vector<std::pair<std::pair<double, double>, unsigned int>> &, // results_points
                         std::map<unsigned int, std::pair<double, double>> &, // results_unique_ids
                         const int &zoom_level);
 
        void nearest_neighbour(KD2Node* ptr, // root
                               const std::pair<double, double> &search_point, // search point
                               double &min_distance, // the minimum distance thus far
                               const std::size_t &depth, // depth of search
                               std::pair<std::pair<double, double>, unsigned int> &results, // results
                               const int &zoom_level); // zoom level
};

// Helper functions to sort vector by X and Y coordinates
bool x_ALessThanB(const std::pair<std::pair<double, double>, unsigned int> &a, const std::pair<std::pair<double, double>, unsigned int> &b);
bool y_ALessThanB(const std::pair<std::pair<double, double>, unsigned int> &a, const std::pair<std::pair<double, double>, unsigned int> &b);

bool xAreEqual(const std::pair<double, double> &a, const std::pair<double, double> &b);
bool yAreEqual(const std::pair<double, double> &a, const std::pair<double, double> &b);

bool depthAreEqual(const std::size_t &depth, const std::pair<double, double> &a, const std::pair<double, double> &b);
bool depthLessThan(const std::size_t &depth, const std::pair<double, double> &a, const std::pair<double, double> &b);

bool depthLessThanBounds(const std::size_t &depth, const std::pair<double, double> &p, const std::pair<double, double> &x, const std::pair<double, double> &y);
bool depthGreaterThanBounds(const std::size_t &depth, const std::pair<double, double> &p, const std::pair<double, double> &x, const std::pair<double, double> &y);

bool depthOverlapsSplit(const std::size_t &depth, const double &min_distance, const std::pair<double, double> &a, const std::pair<double, double> &b);
double pt_dist(const std::pair<double, double> &a, const std::pair<double, double> &b);