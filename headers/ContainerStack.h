#ifndef CONTAINERSTACK_H 
#define CONTAINERSTACK_H

#include <vector>

#include "Container.h"
#include <string>

class ContainerStack {
public:
    ContainerStack(const std::string &name);
    virtual bool push(Container &container);
    Container* pop(); 
    Container* top() const; 
    bool isEmpty(); 
    void printStack();
    int stackOccupancy();
    std::string getName();
    std::vector<Container*> getContainers();
    bool push_infront(Container &container);
protected:
    std::string name;
    std::vector<Container*> containers;
};

#endif // CONTAINERSTACK_H