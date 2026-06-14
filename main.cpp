#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <stdexcept>

#include <Eigen/Dense>
#include "header/autocomplete_gui.h"
#include "header/markov_chain.h"
#include "header/nexttoken_markov_chain.h"

#define EPSILON 0.00001

int main() {
    Eigen::Matrix2d a;
    a << 1.0, 2.0,
         3.0, 4.0;

     Eigen::Matrix<double,6,6> *gamblers_ruin = new Eigen::Matrix<double,6,6>();


     *gamblers_ruin <<
          1.0, 0.5, 0.0, 0.0, 0.0, 0.0,
          0.0, 0.0, 0.5, 0.0, 0.0, 0.0,
          0.0, 0.5, 0.0, 0.5, 0.0, 0.0,
          0.0, 0.0, 0.5, 0.0, 0.5, 0.0,
          0.0, 0.0, 0.0, 0.5, 0.0, 0.0,
          0.0, 0.0, 0.0, 0.0, 0.5, 1.0;
     


     std::cout << "Gamblers Ruin:\n" << *gamblers_ruin << "\n\n";
     MarkovChain<6>* chain = new MarkovChain<6>(gamblers_ruin);
     chain->print_components();
     std::cout << "State vector:\n" << chain->get_state_vector() << "\n\n";



     chain->print_nodes();

     Eigen::Vector<double,6> p;
     p << 0.0, 0.0, 0.0, 1.0, 0.0, 0.0;
     std::cout << "State vector (starting at 3):\n" << chain->get_state_vector(p)<<"\n\n";
     std::cout << "State vector (starting at 3 for 5 rounds):\n" << chain->get_state_vector(p,5)<<"\n\n";


     Eigen::Matrix<double,3,3> *periodic_matrix = new Eigen::Matrix<double,3,3>();
     *periodic_matrix << 
          0.0,0.1,0.0,
          1.0,0.0,1.0,
          0.0,0.9,0.0;

     MarkovChain<3>* periodic_chain = new MarkovChain<3>(periodic_matrix);
     periodic_chain->print_components();
     periodic_chain->print_nodes();
     

     NextTokenMarkovChain autocomplete;
     icu::UnicodeString corpus;

     std::ifstream corpus_file("data/corpus.txt", std::ios::binary);
     if (!corpus_file) {
          throw std::runtime_error("Failed to open data/corpus.txt");
     }

     std::string corpus_utf8((std::istreambuf_iterator<char>(corpus_file)),
                            std::istreambuf_iterator<char>());

     std::replace(corpus_utf8.begin(), corpus_utf8.end(), '\n', ' ');
     std::replace(corpus_utf8.begin(), corpus_utf8.end(), '\r', ' ');
     corpus = icu::UnicodeString::fromUTF8(corpus_utf8);
     autocomplete.set_corpus(corpus);

     AutocompleteGui gui(&autocomplete);
     int exit_code = gui.run();

     delete chain;
     return exit_code;
}
