#ifndef PRIORITYCONTAINER_H
#define PRIORITYCONTAINER_H

class PriorityContainer : public Container {
public:
    PriorityContainer(const std::string &id, int priority, const std::string &destination);
    void displayDetails() const;
    std::string getDetails() const;
private:
    int priority;
    std::string destination;
};

#endif //PRIORITYCONTAINER_H