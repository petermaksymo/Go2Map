/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "KDTree.h"
#include <vector>
#include <iostream>


KD2Node::KD2Node() = default;


KD2Node::KD2Node(const std::pair<double, double> &pt, KD2Node* &left_, KD2Node* &right_, int zoom_level_) {
    point = pt;
    left = left_;
    right = right_;
    zoom_level = zoom_level_;
}

KD2Node::KD2Node(const std::pair<double, double> &pt, int zoom_level_) {
    point = pt;
    zoom_level = zoom_level_;
    left = NULL;
    right = NULL;
}

KD2Node::~KD2Node() {
    delete left;
    delete right;
}

KD2Tree::KD2Tree() = default;

KD2Tree::KD2Tree(std::vector<std::pair<double, double>> pts, const int &zoom_level) {
    
    auto begin = pts.begin();
    auto end = pts.end();
    
    std::size_t depth = 0;
    
    std::size_t length = pts.size();
    
    root = make_tree(begin, end, depth, length, zoom_level);
}

KD2Tree::~KD2Tree() {
    delete root;
}


// Creates balanced 2D KD tree where right is greater than or equal to the node,
// and left is less than or equal to node. Does this by recursively calling itself
// on the left and right parts of the sorted points. Alternates between splitting
// by x and splitting by y.
KD2Node* KD2Tree::make_tree(std::vector<std::pair<double, double>>::iterator begin, // begin
                          std::vector<std::pair<double, double>>::iterator end, // end
                          const std::size_t &depth, // depth
                          const std::size_t &vec_size,
                          const int &zoom_level) {
      
    
    // Vector passed is empty
    if (begin == end) return NULL;
    
    // sort the array by X or Y depending on depth
    if (vec_size > 1) {
        if (depth % 2 == 0) {
            std::sort(begin, end, x_ALessThanB);
        } else {
            std::sort(begin, end, y_ALessThanB);
        }  
    }
    
    // Find middle and 
    auto middle = begin + vec_size / 2;
        
    // Sizes of vectors to left and right of middle
    std::size_t l_size = (vec_size / 2);
    std::size_t r_size = vec_size - l_size - 1;
    
    // Move middle to first point with that value, such that left is always '<' and right is always '>='
    // Adjust size of vector to left and right of middle if middle changes
    bool done = false;
    while(!done && middle != begin) {
        // check if previous median point has equal value in the depth dimension
        if(((depth % 2 == 0) && (xAreEqual(*middle, *(middle - 1)))) || // equal x
            ((depth % 2 == 1) && (yAreEqual(*middle, *(middle - 1))))) { // equal y
            middle--;
            l_size --;
            r_size ++;
        } else {
            done = true;
        }
    }
        
    // make sure that the right node doesn't extend passed the end
    auto r_begin = middle;
    if(middle == end) {
        r_begin = end;
    } else {
        r_begin = middle + 1;
    }
    
    KD2Node* new_node = new KD2Node();
    new_node->point = *middle;
    new_node->zoom_level = zoom_level;
    new_node->left = make_tree(begin, middle, depth + 1, l_size, zoom_level);
    new_node->right = make_tree(r_begin, end, depth + 1, r_size, zoom_level);
    
        
    return new_node;
}

// Recursively visualizes KD Tree horizontally
void KD2Tree::visualize_tree(KD2Node* ptr, const std::size_t depth, const int &zoom_level) {
    if(!ptr) return;
    
    if(ptr->zoom_level > zoom_level) return;

    char dim = 'Y';
    if (depth % 2 == 0) {
        dim = 'X';
    }
    
    for(std::size_t i = 0; i < depth; i++) {
        std::cout << " ";
    }
    
    std::cout << dim << "(" << ptr->point.first << "," << ptr->point.second<< ")" << std::endl;
    visualize_tree(ptr->left, depth + 1, zoom_level);
    visualize_tree(ptr->right, depth + 1, zoom_level);

    return;
}

// Recursive function to insert pair into KD2 Tree
// Tries to insert left if less than middle point, or right if greater than or
// equal to.
// NOTE: Does not work on empty tree
void KD2Tree::insert_pair(KD2Node* ptr, const std::pair<double, double> &new_pt, const std::size_t &depth, const int &zoom_level) {
    if (!ptr) return;

    if(depthLessThan(depth, new_pt, ptr->point)) {
        
        if(ptr->left) insert_pair(ptr->left, new_pt, depth + 1, zoom_level); 
        else ptr->left = new KD2Node(new_pt, zoom_level);
 
    } else {
        
        if(ptr->right) insert_pair(ptr->right, new_pt, depth + 1, zoom_level); 
        else ptr->right = new KD2Node(new_pt, zoom_level);
    }
}

// Inserts an vector of points into an RTree by recursively splitting the array
// to match the nodes in the RTree until places to insert the points are found.
// Makes use of insert_pair and make_tree.
void KD2Tree::insert_bulk(std::vector<std::pair<double, double>>::iterator begin, // begin
                          std::vector<std::pair<double, double>>::iterator end, // end
                          KD2Node* ptr, // root
                          const std::size_t &depth, // depth of insert
                          const std::size_t &vec_size,
                          const int &zoom_level) { // size of passed vector
    // Vector passed is empty
    if (begin == end) return;
    
    // sort the array by X or Y depending on depth
    if (vec_size > 1) {
        if (depth % 2 == 0) {
            std::sort(begin, end, x_ALessThanB);
        } else {
            std::sort(begin, end, y_ALessThanB);
        }  
    }
    
    // if only one pair, can use insert pair function
    if (vec_size == 1) {
        insert_pair(ptr, *begin, depth, zoom_level);
        return;
    }
    
    // Find pick median 
    auto middle = begin + vec_size / 2;
        
    // Sizes of vectors to left and right of middle
    std::size_t l_size = (vec_size / 2);
    std::size_t r_size = vec_size - l_size;
    
    // if middle is less than, move right until middle is greater than or end()
    // if middle is greater than, move right until middle - 1 is less than point or begin()
    if(depthLessThan(depth, *middle, ptr->point)) {
        while(middle != end && depthLessThan(depth, *middle, ptr->point)) {
            middle++;
            l_size++;
            r_size--;
        }
    } else {
        while(middle != begin && !depthLessThan(depth, *(middle -1), ptr->point)) {
            middle--;
            l_size--;
            r_size++;
        }
    }
    
    // If there is another node, call insert bulk with that node as root,
    // otherwise make a new tree with the remainder of the points
    if(ptr->left) insert_bulk(begin, middle, ptr->left, depth  + 1, l_size, zoom_level); 
    else ptr->left = make_tree(begin, middle, depth + 1, l_size, zoom_level);
    
    if(ptr->right) insert_bulk(middle, end, ptr->right, depth  + 1, r_size, zoom_level); 
    else ptr->right = make_tree(middle, end, depth + 1, r_size, zoom_level);
}

// Helper functions for compairing pairs
bool x_ALessThanB(const std::pair<double, double> &a, const std::pair<double, double> &b) {
    return (a.first < b.first);
}
bool y_ALessThanB(const std::pair<double, double> &a, const std::pair<double, double> &b) {
    return (a.second < b.second);
}

bool xAreEqual(const std::pair<double, double> &a, const std::pair<double, double> &b) {
    return (a.first == b.first);
}

bool yAreEqual(const std::pair<double, double> &a, const std::pair<double, double> &b) {
    return (a.second == b.second);
}

bool depthLessThan(const std::size_t &depth, const std::pair<double, double> &a, const std::pair<double, double> &b) {
    return (((depth % 2 == 0) && (a.first < b.first)) || ((depth % 2 == 1) && (a.second < b.second)));
}