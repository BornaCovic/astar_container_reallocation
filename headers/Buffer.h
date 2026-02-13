#ifndef BUFFER_H
#define BUFFER_H

#include "ContainerStack.h"

class Buffer : public ContainerStack {
public:
    Buffer(int size, const std::string &name);
    bool isFull();
    bool push(Container &container) override;
    int getSize();
private:
    int size;
};

#endif // BUFFER_H