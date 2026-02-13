#include "AStarStartingState.h"
#include <iostream>

static int untilDueToSeconds(UntilDue& ud) {
    return ud.getMinutes() * 60 + ud.getSeconds();
}

AStarState makeAStarInitialState(ParsedBuffers& parsedBuffers) {
    return makeAStarCurrentState(parsedBuffers, 0, nullptr);
}

AStarState makeAStarCurrentState(ParsedBuffers& parsedBuffers, int currentSystemTime, SingleContainerCrane* crane) {
    AStarState state;
    state.current_time = currentSystemTime;
    state.consecutiveWaits = 0;
    state.totalWaitTime = 0;
    
    if (crane != nullptr) {
        state.crane.position = crane->getAboveStackIndex();
        state.crane.hasContainer = (crane->getHookContent() != nullptr);
        if (state.crane.hasContainer) {
            void* hookContent = crane->getHookContent();
            UntilDueContainer* container = static_cast<UntilDueContainer*>(hookContent);
            if (container) {
                state.crane.containerId = container->getId();
            }
        }
    } else {
        state.crane.position = 0;
        state.crane.hasContainer = false;
        state.crane.containerId = "";
    }

    auto buffers = parsedBuffers.getBuffers();
    
    for (size_t bufferIndex = 0; bufferIndex < buffers.size(); bufferIndex++) {
        std::vector<UntilDueContainer> stack;
        
        if(bufferIndex != 4){
            for (auto containerPtr : buffers[bufferIndex]->getContainers()) {
                auto udc = dynamic_cast<UntilDueContainer*>(containerPtr);
                if (udc) {
                    UntilDueContainer container = *udc;
                    
                    UntilDue ud = udc->getUntilDue();
                    int dueInSeconds = untilDueToSeconds(ud);
                    
                    container.setDueIn(dueInSeconds + currentSystemTime);
                    
                    container.setArrivalTime(0);
                    
                    container.setExitTime(-1);
                    
                    stack.push_back(container);
                }
            }
        }
        
        state.stacks.push_back(stack);
    }
    
    // If crane is carrying a container, we need to add it to the state
    if (crane != nullptr && state.crane.hasContainer) {
        std::cout << "Crane is currently carrying container: " << state.crane.containerId << std::endl;
    }
    
    #ifdef DEBUG
    printCurrentStateInfo(state, currentSystemTime);
    #endif
    
    return state;
}

// Helper function to update all container due times when time passes
void updateContainerDueTimes(AStarState& state, int elapsedTime) {
    for (auto& stack : state.stacks) {
        for (auto& container : stack) {
            if (container.getExitTime() == -1) {  // Only update unexited containers
                // Get current due time
                int currentDueIn = container.getDueIn();
                
                // Subtract elapsed time
                currentDueIn -= elapsedTime;
                
                // Handle negative time (overdue)
                if (currentDueIn < 0) {
                    currentDueIn = 0;  // Container is overdue
                }
                
                // Update the container's due time
                container.setDueIn(currentDueIn);
                
                // Also update the UntilDue object for consistency
                int newMinutes = currentDueIn / 60;
                int newSeconds = currentDueIn % 60;
                UntilDue newDue(newMinutes, newSeconds);
                container.setUntilDue(newDue);
            }
        }
    }
    
    state.current_time += elapsedTime;
}

// Debug helper to print current state information
void printCurrentStateInfo(const AStarState& state, int systemTime) {
    std::cout << "\n=== Current A* State ===" << std::endl;
    std::cout << "System time: " << systemTime << " seconds" << std::endl;
    std::cout << "A* state time: " << state.current_time << " seconds" << std::endl;
    std::cout << "Crane position: Stack " << state.crane.position << std::endl;
    std::cout << "Crane has container: " << (state.crane.hasContainer ? "Yes" : "No") << std::endl;
    if (state.crane.hasContainer) {
        std::cout << "Crane container ID: " << state.crane.containerId << std::endl;
    }
    
    std::cout << "\nStacks configuration:" << std::endl;
    for (size_t i = 0; i < state.stacks.size(); i++) {
        std::cout << "Stack " << i << " (";
        
        if (i == 0) {
            std::cout << "Entry";
        } else if (i == state.stacks.size() - 1) {
            std::cout << "Outgoing";
        } else {
            std::cout << "Buffer";
        }
        
        std::cout << "): " << state.stacks[i].size() << " containers" << std::endl;
        
        for (size_t j = 0; j < state.stacks[i].size(); j++) {
            const auto& container = state.stacks[i][j];
            int absoluteDueTime = state.current_time + container.getDueIn();
            std::cout << "  [" << j << "] " << container.getId() 
                      << " (due in: " << container.getDueIn() << "s, "
                      << "absolute due time: " << absoluteDueTime << "s";
            
            if (container.getDueIn() <= 0) {
                std::cout << " - OVERDUE!";
            }
            
            std::cout << ")" << std::endl;
        }
    }
    std::cout << "========================\n" << std::endl;
}

void printInitialStateInfo(const AStarState& state) {
    printCurrentStateInfo(state, 0);
}