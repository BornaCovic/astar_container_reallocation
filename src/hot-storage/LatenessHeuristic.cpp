#include "/home/borna/ZavrsniRad/headers/LatenessHeuristic.h"
#include <algorithm>
#include <cmath>
#include <iostream>

LatenessHeuristic::LatenessHeuristic(const ParsedBuffers& buffers) {
    UntilDue moveTime = buffers.getCraneMove();
    craneMoveTime = moveTime.getMinutes() * 60 + moveTime.getSeconds();
    
    UntilDue lowerTime = buffers.getCraneLower();
    craneLowerTime = lowerTime.getMinutes() * 60 + lowerTime.getSeconds();
    
    UntilDue liftTime = buffers.getCraneLift();
    craneLiftTime = liftTime.getMinutes() * 60 + liftTime.getSeconds();
    
    UntilDue clearTime = buffers.getClearingTime();
    clearingTime = clearTime.getMinutes() * 60 + clearTime.getSeconds();
    
    #ifdef DEBUG
    std::cout << "LatenessHeuristic initialized with:" << std::endl;
    std::cout << "  Move time: " << craneMoveTime << "s" << std::endl;
    std::cout << "  Lower time: " << craneLowerTime << "s" << std::endl;
    std::cout << "  Lift time: " << craneLiftTime << "s" << std::endl;
    std::cout << "  Clearing time: " << clearingTime << "s" << std::endl;
    #endif
}

double LatenessHeuristic::evaluate(const AStarState& state) const {
    return calculateMinimumLateness(state);
}

double LatenessHeuristic::calculateMinimumLateness(const AStarState& state) const {
    double totalLateness = 0.0;
    
        if (state.crane.hasContainer) {
                for (const auto& stack : state.stacks) {
            for (const auto& container : stack) {
                if (container.getId() == state.crane.containerId && 
                    container.getExitTime() == -1) {
                    
                                        int outgoingStackIndex = state.stacks.size() - 1;
                    int moveDistance = std::abs(outgoingStackIndex - state.crane.position);
                    int timeToExit = moveDistance * craneMoveTime + craneLowerTime;
                    
                                        int exitTime = ((state.current_time + timeToExit));
                    
                                        int dueTime = container.getArrivalTime() + container.getDueIn();
                    
                                        int lateness = std::max(0, exitTime - dueTime);
                    totalLateness += lateness;

                    
                    #ifdef DEBUG
                    if (lateness > 0) {
                        std::cout << "  Held container " << container.getId() 
                                  << " lateness: " << lateness << std::endl;
                    }
                    #endif
                    
                    break;
                }
            }
        }
    }
    
        for (size_t stackIdx = 1; stackIdx < state.stacks.size() - 1; stackIdx++) {
        const auto& stack = state.stacks[stackIdx];
        
                for (size_t pos = 0; pos < stack.size(); pos++) {
            const auto& container = stack[pos];
            
                        if (container.getExitTime() != -1) {
                continue;
            }
            
                        int minTimeToExit = calculateMinTimeToExit(state, stackIdx, pos);

            if(state.crane.hasContainer){
                int outgoingStackIndex = state.stacks.size() - 1;
                int moveDistance = std::abs(outgoingStackIndex - state.crane.position);
                int timeToExit = moveDistance * craneMoveTime + craneLowerTime;
                minTimeToExit += timeToExit;
            }
            
                        int exitTime = (state.current_time + minTimeToExit);
            
                        int dueTime = container.getArrivalTime() + container.getDueIn();
            
                        int lateness = std::max(0, exitTime - dueTime);
            totalLateness += lateness;
            
            #ifdef DEBUG
            if (lateness > 0) {
                std::cout << "  Container " << container.getId() 
                          << " at stack " << stackIdx << " pos " << pos
                          << " lateness: " << lateness 
                          << " (exits at " << exitTime << ", due at " << dueTime << ")"
                          << std::endl;
            }
            #endif
        }
    }
    
        if (!state.stacks[0].empty()) {
        const auto& entryStack = state.stacks[0];
        for (size_t pos = 0; pos < entryStack.size(); pos++) {
            const auto& container = entryStack[pos];
            
            if (container.getExitTime() != -1) {
                continue;
            }
            
                        int minTimeToExit = calculateMinTimeToExit(state, 0, pos);
            int exitTime = (state.current_time + minTimeToExit);
            int dueTime = container.getArrivalTime() + container.getDueIn();
            int lateness = std::max(0, exitTime - dueTime);
            totalLateness += lateness;
        }
    }

    
    return totalLateness;
}

int LatenessHeuristic::calculateMinTimeToExit(const AStarState& state, 
                                              int stackIndex, 
                                              int containerPosition) const {
    int totalTime = 0;
    int outgoingStackIndex = state.stacks.size() - 1;
    
        if (!state.crane.hasContainer && state.crane.position != stackIndex) {
        totalTime += getMinMovesBetweenStacks(state.crane.position, stackIndex) * craneMoveTime;
    }
    
        int containersAbove = state.stacks[stackIndex].size() - containerPosition - 1;
    if (containersAbove > 0) {
                                                                
        int perContainerTime = craneLowerTime + craneLiftTime + 
                              craneMoveTime + craneLowerTime + craneLiftTime + 
                              craneMoveTime;
        
        totalTime += containersAbove * perContainerTime;
    }
    
        totalTime += craneLowerTime + craneLiftTime;
    
        totalTime += getMinMovesBetweenStacks(stackIndex, outgoingStackIndex) * craneMoveTime;
    
        totalTime += craneLowerTime;
    
        
    return totalTime;
}

int LatenessHeuristic::getMinMovesBetweenStacks(int from, int to) const {
    return std::abs(to - from);
}