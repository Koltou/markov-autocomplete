#pragma once 
#ifndef NODE_H
#define NODE_H
#include <string>
struct Node {
    int id=-1;
    int component_id=-1;
    std::string type="recurrent";//transient or recurrent
    unsigned int periodicity=0;
};
#endif 