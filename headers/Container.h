#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <string>

class Container {
    public:
        Container();
        Container(const std::string &id);
        virtual ~Container();
        virtual void displayDetails() const = 0;
        virtual std::string getDetails() const = 0;
    protected:
        std::string id;
};

#endif // CONTAINERS_H