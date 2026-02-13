#ifndef ENTRYCONTAINERSTACK_H
#define ENTRYCONTAINERSTACK_H

#include <iostream>
#include <string>
#include <chrono>
#include <cstdlib>
#include <functional>  // For std::function
#include "Buffer.h"
#include "UntilDueContainer.h"

class EntryContainerStack : public Buffer {
private:
    int containerId; 
    int pauseFlag;
    std::function<void()> onContainerAdded;  // Callback function
    
public:
    EntryContainerStack();
    void startAutoAddContainers(int iterations, double delayInSeconds);
    void pauseTime();
    void continueTime();
    int &getPauseFlag();
    
    // New method to set the callback
    void setOnContainerAddedCallback(std::function<void()> callback);
    
    // Note: We'll call the callback directly in startAutoAddContainers
    // since Buffer::push is not virtual
};

#endif // ENTRYCONTAINERSTACK_H