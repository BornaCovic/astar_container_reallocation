#include <string>
#include <vector>

#include "Buffer.h"
#include "Container.h"
#include "UntilDueContainer.h"

Buffer::Buffer(int size, const std::string &name) : ContainerStack(name), size(size)  {}

bool Buffer::isFull(){
    if(containers.size()>=size){
        return true;
    }
    return false;
}
        
bool Buffer::push(Container &container) {
    if(!isFull()){
        containers.push_back(&container);
        return true;
    }
    return false;
}

int Buffer::getSize(){
    return size;
}