#include "StateGenerator.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <climits>
#include <thread>
#include <chrono>




StateGenerator::StateGenerator(const ParsedBuffers& buffers) : buffers(buffers) {
        UntilDue moveTime = buffers.getCraneMove();
    craneMoveTime = moveTime.getMinutes() * 60 + moveTime.getSeconds();
    std::cout<< "Crane Move Time: " << craneMoveTime << "\n";
    
    UntilDue lowerTime = buffers.getCraneLower();
    craneLowerTime = lowerTime.getMinutes() * 60 + lowerTime.getSeconds();
    std::cout<< "Crane Lower Time: " << craneLowerTime << "\n";
    
    UntilDue liftTime = buffers.getCraneLift();
    craneLiftTime = liftTime.getMinutes() * 60 + liftTime.getSeconds();
    std::cout<< "Crane Lift Time: " << craneLiftTime << "\n";
    
    UntilDue clearTime = buffers.getClearingTime();
    clearingTime = clearTime.getMinutes() * 60 + clearTime.getSeconds();
    std::cout<< "Clearing Time: " << clearingTime << "\n";
}

std::vector<std::pair<AStarState, double>> 
StateGenerator::generateSuccessors(const AStarState& current) const {
    std::vector<std::pair<AStarState, double>> successors;

    if (!current.crane.hasContainer) {
                for (size_t i = 0; i < current.stacks.size(); i++) {
            if (current.canPickUpFrom(i)) {
                double cost;
                AStarState newState = applyPickUp(current, i, cost);
                successors.push_back({newState, cost});
            }
        }
    } else {
                for (size_t i = 0; i < current.stacks.size(); i++) {
            if (current.canPutDownOn(i, buffers.getBufferSize())) {
                double cost;
                AStarState newState = applyPutDown(current, i, cost);
                successors.push_back({newState, cost});
            }
        }
    }

        if (shouldConsiderWaiting(current)) {
        int waitTime = calculateOptimalWaitTime(current);
        if (waitTime > 0 && waitTime <= MAX_WAIT_TIME) {
            double waitCost;
            AStarState waitedState = applyWait(current, waitTime, waitCost);
            successors.push_back({waitedState, waitCost});
        }
    }
/*for (const auto& succ : successors) {
    const AStarState& state = succ.first;
    std::cout << "==== Generated State ====\n";
    std::cout << "Action: " << state.lastAction << "\n";
    std::cout << "Current time: " << state.current_time << " seconds\n";
    std::cout << "Crane position: " << state.crane.position 
              << " | Holding: " << (state.crane.hasContainer ? "YES" : "NO") << "\n";
    if (state.crane.hasContainer) {
        std::cout << "  Held container ID: " << state.crane.containerId << "\n";
    }
    for (size_t i = 0; i < state.stacks.size(); ++i) {
        std::cout << "Stack " << i << ": ";
        for (const auto& cont : state.stacks[i]) {
            std::cout << cont.getId() << "(due:" << cont.getDueIn()
                      << ", exit:" << cont.getExitTime() << ") ";
        }
        std::cout << "\n";
    }
    std::cout << "========================\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
}*/

    return successors;
}

bool StateGenerator::shouldConsiderWaiting(const AStarState& current) const {
        if (!canWaitingHelp(current)) {
        return false;
    }
    
        if (hasWaitedTooMuch(current)) {
        return false;
    }
    
    return true;
}

bool StateGenerator::canWaitingHelp(const AStarState& current) const {
    auto& exitStack = current.stacks.back();
    
        if (!exitStack.empty()) {
                const auto& topContainer = exitStack.back();
        if (topContainer.getExitTime() != -1) {
            int clearTime = topContainer.getExitTime() + clearingTime;
            int timeUntilClear = clearTime - current.current_time;
            
                        if (timeUntilClear > 0 && timeUntilClear <= MAX_WAIT_TIME) {
                return true;
            }
        }
    }
    
        if (current.crane.hasContainer && !exitStack.empty()) {
                const auto& topContainer = exitStack.back();
        if (topContainer.getExitTime() != -1) {
            int clearTime = topContainer.getExitTime() + clearingTime;
            if (clearTime - current.current_time > 0 && 
                clearTime - current.current_time <= MAX_WAIT_TIME) {
                return true;
            }
        }
    }
    
            bool hasUrgentWork = false;
    for (size_t i = 0; i < current.stacks.size() - 1; i++) {          if (!current.stacks[i].empty()) {
            const auto& top = current.stacks[i].back();
            if (top.getExitTime() == -1) {                  int dueTime = top.getArrivalTime() + top.getDueIn();
                if (dueTime < current.current_time) {
                    hasUrgentWork = true;
                    break;
                }
            }
        }
    }
    
        return !hasUrgentWork;
}

bool StateGenerator::hasWaitedTooMuch(const AStarState& current) const {
        if (current.consecutiveWaits >= MAX_CONSECUTIVE_WAITS) {
        return true;
    }
    
        if (current.current_time > 0) {
        double waitRatio = static_cast<double>(current.totalWaitTime) / current.current_time;
        if (waitRatio > MAX_WAIT_RATIO) {
            return true;
        }
    }
    
    return false;
}

int StateGenerator::calculateOptimalWaitTime(const AStarState& current) const {
    int minWaitTime = INT_MAX;
    auto& exitStack = current.stacks.back();
    
        if (!exitStack.empty()) {
                const auto& topContainer = exitStack.back();
        if (topContainer.getExitTime() != -1) {
            int clearTime = topContainer.getExitTime() + clearingTime;
            int timeUntilClear = clearTime - current.current_time;
            
            if (timeUntilClear > 0) {
                minWaitTime = timeUntilClear;
            }
        }
    }
    
        return std::min(minWaitTime, MAX_WAIT_TIME);
}


std::vector<Action> StateGenerator::getValidActions(const AStarState& current) const {
    std::vector<Action> actions;
    
    if (!current.crane.hasContainer) {
                for (size_t i = 0; i < current.stacks.size(); i++) {
            if (current.canPickUpFrom(i)) {
                std::string desc = "Pick up " + 
                    current.stacks[i].back().getId() + 
                    " from stack " + std::to_string(i);
                actions.push_back(Action(Action::PICK_UP, i, desc));
            }
        }
    } else {
                for (size_t i = 0; i < current.stacks.size(); i++) {
            if (current.canPutDownOn(i, buffers.getBufferSize())) {
                std::string desc = "Put down " + 
                    current.crane.containerId + 
                    " on stack " + std::to_string(i);
                actions.push_back(Action(Action::PUT_DOWN, i, desc));
            }
        }
    }
    
        if (shouldConsiderWaiting(current)) {
        int waitTime = calculateOptimalWaitTime(current);
        if (waitTime > 0) {
            std::string desc = "Wait for " + std::to_string(waitTime) + " seconds";
            actions.push_back(Action(Action::WAIT, -1, desc, waitTime));
        }
    }
    
    return actions;
}

AStarState StateGenerator::applyPickUp(const AStarState& current, 
                                       int stackIndex, 
                                       double& cost) const {
    AStarState newState = current;
    cost = 0;
    
        newState.consecutiveWaits = 0;
    
        if (current.crane.position != stackIndex) {
        int moveTime = calculateCraneMoveTime(current.crane.position, stackIndex);
        cost += moveTime;
        newState.current_time += moveTime;
        newState.crane.position = stackIndex;
        
                updateAllContainerTimes(newState, moveTime);
        clearExitedContainers(newState, moveTime);
    }
    
        int pickUpTime = craneLowerTime + craneLiftTime;
    cost += pickUpTime;
    newState.current_time += pickUpTime;
    
        UntilDueContainer pickedContainer = newState.stacks[stackIndex].back();
    newState.stacks[stackIndex].pop_back();
    
        newState.crane.hasContainer = true;
    newState.crane.containerId = pickedContainer.getId();
    newState.crane.heldContainer = pickedContainer;
    
        updateAllContainerTimes(newState, pickUpTime);
    clearExitedContainers(newState, pickUpTime);
    
        newState.lastAction = "Picked up " + pickedContainer.getId() + 
                         " from stack " + std::to_string(stackIndex);
    newState.accumulatedCost = current.accumulatedCost + cost;
    
    #ifdef DEBUG
    std::cout << "Generated successor: " << newState.lastAction 
              << " (cost: " << cost << ")" << std::endl;
    #endif
    
    return newState;
}

AStarState StateGenerator::applyPutDown(const AStarState& current, 
                                        int stackIndex, 
                                        double& cost) const {
    AStarState newState = current;
    cost = 0;
    
        newState.consecutiveWaits = 0;

    int accumulatedLateness = current.getTotalLateness();
    
        if (current.crane.position != stackIndex) {
        int moveTime = calculateCraneMoveTime(current.crane.position, stackIndex);
        cost += moveTime;
        newState.current_time += moveTime;
        newState.crane.position = stackIndex;
        
                updateAllContainerTimes(newState, moveTime);
        clearExitedContainers(newState, moveTime);
    }
    
        int putDownTime = craneLowerTime;
    cost += putDownTime;
    newState.current_time += putDownTime;
    
        if (!current.crane.heldContainer.has_value()) {
        std::cerr << "[ERROR] Crane is not holding any container in applyPutDown(). Aborting!" << std::endl;
        abort();
    }
    UntilDueContainer newContainer = current.crane.heldContainer.value();
    
        if (stackIndex == static_cast<int>(newState.stacks.size()) - 1) {
                int nextBoundary = ((newState.current_time / 60) + 1) * 60;
        
                int existingContainers = newState.stacks[stackIndex].size();
        
                newContainer.setExitTime(nextBoundary);

        if ((newState.current_time - newContainer.getDueIn()) > 0){
            newState.setTotalLateness(accumulatedLateness + newState.current_time - newContainer.getDueIn());
        }
        
        
                for (size_t i = 0; i < newState.stacks[stackIndex].size(); i++) {
                        int containerClearTime = nextBoundary + ((existingContainers - i) * 60);
            newState.stacks[stackIndex][i].setExitTime(containerClearTime);
        }
    }
    
        newState.stacks[stackIndex].push_back(newContainer);
    
        newState.crane.hasContainer = false;
    newState.crane.containerId = "";
    newState.crane.heldContainer = std::nullopt;
    
        int liftTime = craneLiftTime;
    cost += liftTime;
    newState.current_time += liftTime;
    
        updateAllContainerTimes(newState, putDownTime + liftTime);
    clearExitedContainers(newState, putDownTime + liftTime);
    
        newState.lastAction = "Put down " + newContainer.getId() + 
                         " on stack " + std::to_string(stackIndex);
    if (stackIndex == static_cast<int>(newState.stacks.size()) - 1) {
        newState.lastAction += " (EXIT)";
    }
    newState.accumulatedCost = current.accumulatedCost + cost;
    
    return newState;
}

void StateGenerator::updateAllContainerTimes(AStarState& state, int elapsedTime) const {
    for (auto& stack : state.stacks) {
        for (auto& container : stack) {
            if (container.getExitTime() == -1) {                                  int totalSeconds = container.getUntilDue().getMinutes() * 60 + 
                                  container.getUntilDue().getSeconds();
                totalSeconds -= elapsedTime;
                
                                int newMinutes = totalSeconds / 60;
                int newSeconds = totalSeconds % 60;
                
                                if (totalSeconds < 0) {
                    newMinutes = -((-totalSeconds) / 60);
                    newSeconds = -((-totalSeconds) % 60);
                }
                
                UntilDue newUntilDue(newMinutes, newSeconds);
                container.setUntilDue(newUntilDue);
                
                            }
        }
    }
}

void StateGenerator::clearExitedContainers(AStarState& state, int time) const {
    auto& exitStack = state.stacks.back(); 
        while (!exitStack.empty()) {
        const auto& topContainer = exitStack.back();
                if (topContainer.getExitTime() != -1 &&
            topContainer.getExitTime() <= state.current_time) {
                        exitStack.pop_back();
        } else {
            break;
        }
    }
}

AStarState StateGenerator::applyWait(const AStarState& current, int waitTime, double& cost) const {
    AStarState newState = current;
    cost = waitTime;
    
        newState.consecutiveWaits = current.consecutiveWaits + 1;
    newState.totalWaitTime = current.totalWaitTime + waitTime;
    
    newState.current_time += waitTime;

        updateAllContainerTimes(newState, waitTime);
    clearExitedContainers(newState, waitTime);

    newState.lastAction = "Waited for " + std::to_string(waitTime) + " seconds";
    newState.accumulatedCost = current.accumulatedCost + cost;

    return newState;
}


int StateGenerator::calculateCraneMoveTime(int from, int to) const {
    return std::abs(to - from) * craneMoveTime;
}