#ifndef ASTAR_STATE_H
#define ASTAR_STATE_H

#include <optional>
#include <vector>
#include <string>
#include <iostream>
#include "UntilDueContainer.h"

struct CraneState {
    int position;
    bool hasContainer;
    std::string containerId;

        std::optional<UntilDueContainer> heldContainer;

        CraneState() : position(0), hasContainer(false), containerId(""), heldContainer(std::nullopt) {}

        std::string toString() const {
        std::string result = "Crane at stack " + std::to_string(position);
        if (hasContainer) {
            result += " holding " + (containerId.empty() ? "???" : containerId);
        } else {
            result += " (empty)";
        }
        return result;
    }

        const UntilDueContainer* getHeldContainer() const {
        return hasContainer && heldContainer.has_value() ? &heldContainer.value() : nullptr;
    }
};

struct AStarState {
                    std::vector<std::vector<UntilDueContainer>> stacks;
    
        int current_time;
    
        CraneState crane;
    
        std::string lastAction;
    double accumulatedCost;

    int consecutiveWaits;
    int totalWaitTime;
    int nextClearingTime;  
    
        struct ClearedContainer {
        std::string id;
        int clearedAtTime;
        int lateness;
    };
    std::vector<ClearedContainer> clearedContainers;
    double totalAccumulatedLateness;      
        AStarState() : current_time(0), accumulatedCost(0), 
                   consecutiveWaits(0), totalWaitTime(0), 
                   totalAccumulatedLateness(0),
                   lastAction("Initial state") {}
    
        std::string getStateHash() const;
    
        bool isGoalState() const;
    
        void printState() const;
    
        int getTotalContainers() const;
    
        int getUnexitedContainers() const;
    
            std::pair<int, int> findContainer(const std::string& containerId) const;
    
        bool canPickUpFrom(int stackIndex) const;
    bool canPutDownOn(int stackIndex, int bufferSize) const;
    
        double getTotalLateness() const;
    void setTotalLateness(int late);
    
        UntilDueContainer* getTopContainer(int stackIndex);
    const UntilDueContainer* getTopContainer(int stackIndex) const;
    
        bool operator==(const AStarState& other) const;
};

#endif 