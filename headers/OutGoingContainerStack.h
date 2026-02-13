#ifndef OUTGOINGCONTAINERSTACK_H
#define OUTGOINGCONTAINERSTACK_H

#include <iostream>
#include <string>
#include <chrono>
#include "Buffer.h"

class OutGoingContainerStack : public Buffer {
private:
    int pauseFlag;

public:
    OutGoingContainerStack();
    void startPoppingContainers(int popDelay);
    
    void continueTime(){
        pauseFlag = 0;
    };

    void pauseTime(){
        pauseFlag = 1;
    };
};

#endif // OUTGOINGCONTAINERSTACK_H
