#pragma once
#ifndef EPSILON
#define EPSILON 0.00001
#endif
#ifndef MARKOV_CHAIN_H 
#define MARKOV_CHAIN_H
#include <Eigen/Dense>
#include <cmath>
#include <array>
#include <string>
#include <stack>
#include <unordered_set>
#include <vector>
#include <queue>
#include <numeric>
#include "header/node.h"
#include "header/linear_algebra_utils.h"
template <int N >
class MarkovChain {
    public:
        MarkovChain(Eigen::Matrix<double,N,N> *M);
        ~MarkovChain();

        void print_components();
        void print_nodes();
        Eigen::Vector<double,N> get_state_vector(long long n=1000);
        Eigen::Vector<double,N> get_state_vector(const Eigen::Vector<double,N>& p,long long n=1000);

        unsigned int get_periodicity(unsigned int i);
        bool is_aperiodic();
        bool is_irreducible();
        std::vector<unsigned int> get_neighbors(unsigned int i);
        std::vector<unsigned int> get_neighbors_within_scc(unsigned int node_i,unsigned int component_i);

        std::vector<std::pair<unsigned int,unsigned int>> get_edges();
        std::vector<std::pair<unsigned int,unsigned int>> get_edges_within_scc(unsigned int i);
  

    private:
        bool is_schotastic();
        void kosaraju_scc();
        void depth_first_search(const Eigen::Matrix<double,N,N> & M,std::stack<int>& stack,std::unordered_set<int>&visited,int i=0);
        std::vector<int> get_destinations(const Eigen::Matrix<double,N,N>& M,int i);
        std::vector<std::unordered_set<int>> get_scc(const Eigen::Matrix<double,N,N>&M,std::stack<int>&to_visit);        
        void set_nodes_periodicity();
        void set_scc_periodicity(unsigned int i );
        void set_node_types();
        



        Eigen::Matrix<double,N,N>* M;
        std::array<Node,N> nodes; 
        std::vector<std::unordered_set<int>> scc;

        bool is_aperiodic_val;
        bool is_aperiodic_calculated;
};

    #include "header/markov_chain.tpp"

#endif 