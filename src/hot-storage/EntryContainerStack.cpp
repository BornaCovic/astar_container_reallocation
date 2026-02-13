#include <iostream>
#include <string>
#include <chrono>
#include <cstdlib>
#include "EntryContainerStack.h"

EntryContainerStack::EntryContainerStack() : containerId(1), Buffer(20, "A0") {
    pauseTime();
    onContainerAdded = nullptr;  // Initialize callback to null
}

void EntryContainerStack::pauseTime(){
    pauseFlag = 1;
}

void EntryContainerStack::continueTime(){
    pauseFlag = 0;
}

// New method to set the callback
void EntryContainerStack::setOnContainerAddedCallback(std::function<void()> callback) {
    onContainerAdded = callback;
}

// Note: Since Buffer::push is not virtual, we call the callback 
// directly in startAutoAddContainers instead of overriding push

void EntryContainerStack::startAutoAddContainers(int iterations, double delayInSeconds) {
    for (int i = 0; i < iterations; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        while (elapsed.count() < delayInSeconds) {
            if(pauseFlag == 0){
            end = std::chrono::high_resolution_clock::now();
            elapsed = end - start;
            }
        }

        int containerId = std::rand() % 100 + 1;
        int randMinutes = std::rand() % 4;
        int randSeconds = std::rand() % 60;

        std::string name = "B";
        name.append(std::to_string(containerId));

        UntilDue randUntilDue(randMinutes, randSeconds);
        Container* newContainer = new UntilDueContainer(name, randUntilDue);

        // Add the container using the base class method
        Buffer::push(*newContainer);
        
        // Trigger the callback if it's set
        if (onContainerAdded) {
            onContainerAdded();
        }
    }
}

int &EntryContainerStack::getPauseFlag(){
    return pauseFlag;
}