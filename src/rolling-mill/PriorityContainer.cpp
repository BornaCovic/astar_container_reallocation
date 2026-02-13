#include <iostream>
#include <string>

#include "Container.h"
#include "PriorityContainer.h"

PriorityContainer::PriorityContainer(const std::string &id, int priority, const std::string &destination)
        : Container(id), priority(priority), destination(destination) {}

void PriorityContainer::displayDetails() const {
        std::cout << id << " " << priority << " " << destination << std::endl;
}

std::string PriorityContainer::getDetails() const{
        return id + " " + std::to_string(priority) + " " + destination;
}