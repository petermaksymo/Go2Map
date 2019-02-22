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
        
        
        KD2Node();
        KD2Node(const std::pair<double, double> &pt);
        KD2Node(const std::pair<double, double> &, KD2Node* &, KD2Node* &);
        ~KD2Node();
};

class KD2Tree {
    public:
        KD2Node* root;
        
        KD2Tree();
        KD2Tree(std::vector<std::pair<double, double>>);
        ~KD2Tree();
        KD2Node* make_tree(std::vector<std::pair<double, double>>::iterator, // begin
                          std::vector<std::pair<double, double>>::iterator, // end
                          const std::size_t &, // depth
                          const std::size_t &); // size of passed vector
        void visualize_tree(KD2Node* root, const std::size_t &);
};

// Helper functions to sort vector by X and Y coordinates
bool sortByX(const std::pair<double, double> &a, const std::pair<double, double> &b);
bool sortByY(const std::pair<double, double> &a, const std::pair<double, double> &b);

bool xAreEqual(const std::pair<double, double> &a, const std::pair<double, double> &b);
bool yAreEqual(const std::pair<double, double> &a, const std::pair<double, double> &b);

