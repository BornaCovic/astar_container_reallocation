#include <chrono>
#include <thread> 

#include "OutGoingContainerStack.h"

OutGoingContainerStack::OutGoingContainerStack()
    : Buffer(10, "H0") {
        pauseTime();
    }

void OutGoingContainerStack::startPoppingContainers(int popDelay) {
    while (!isEmpty()) {
        auto start = std::chrono::high_resolution_clock::now();
        auto end = start;
        std::chrono::duration<double> elapsed = end - start;

        while (elapsed.count() < popDelay) {
            if (pauseFlag == 0) {
                end = std::chrono::high_resolution_clock::now();
                elapsed = end - start;
            }
        }

        Container* poppedContainer = this->pop();
        if (poppedContainer) {
            delete poppedContainer;
        }
    }
}
