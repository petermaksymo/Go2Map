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

class KD2Node {
    public:
        std::pair<double, double> point; // [x, y]
        KD2Node* left; // less than
        KD2Node* right; // greater than or equal
        int zoom_level;
        
        
        KD2Node();
        KD2Node(const std::pair<double, double> &pt, int zoom_level);
        KD2Node(const std::pair<double, double> &, KD2Node* &, KD2Node* &, int zoom_level);
        ~KD2Node();
};

class KD2Tree {
    public:
        KD2Node* root;
        
        KD2Tree();
        KD2Tree(std::vector<std::pair<double, double>>, const int &zoom_level);
        ~KD2Tree();
        
        KD2Node* make_tree(std::vector<std::pair<double, double>>::iterator, // begin
                          std::vector<std::pair<double, double>>::iterator, // end
                          const std::size_t &, // depth
                          const std::size_t &, // size of passed vector
                          const int &zoom_level);
        
        void visualize_tree(KD2Node*, const std::size_t, const int &zoom_level);
        
        void insert_pair(KD2Node*, const std::pair<double, double> &, const std::size_t &, const int &zoom_level);
        
        void insert_bulk(std::vector<std::pair<double, double>>::iterator, // begin
                          std::vector<std::pair<double, double>>::iterator, // end
                          KD2Node*, // root
                          const std::size_t &, // depth of insert
                          const std::size_t &, // size of passed vector
                          const int &zoom_level); // zoom level flag
        
        void range_query(KD2Node*, // root
                         const std::size_t &depth, // depth of query
                         const std::pair<double, double> &, // x-range (smaller, greater)
                         const std::pair<double, double> &, // y-range (smaller, greater)
                         std::vector<std::pair<double, double>> &, // results
                         const int &zoom_level);
};

// Helper functions to sort vector by X and Y coordinates
bool x_ALessThanB(const std::pair<double, double> &a, const std::pair<double, double> &b);
bool y_ALessThanB(const std::pair<double, double> &a, const std::pair<double, double> &b);

bool xAreEqual(const std::pair<double, double> &a, const std::pair<double, double> &b);
bool yAreEqual(const std::pair<double, double> &a, const std::pair<double, double> &b);

bool depthLessThan(const std::size_t &depth, const std::pair<double, double> &a, const std::pair<double, double> &b);

bool depthLessThanBounds(const std::size_t &depth, const std::pair<double, double> &p, const std::pair<double, double> &x, const std::pair<double, double> &y);
bool depthGreaterThanBounds(const std::size_t &depth, const std::pair<double, double> &p, const std::pair<double, double> &x, const std::pair<double, double> &y);

