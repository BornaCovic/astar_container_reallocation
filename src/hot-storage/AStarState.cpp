#include "AStarState.h"
#include <sstream>
#include <algorithm>
#include <iomanip>

std::string AStarState::getStateHash() const {
    std::stringstream ss;

    // Include crane state
    ss << crane.position << "|";
    ss << (crane.getHeldContainer() ? "1" : "0") << "|";
    if (crane.getHeldContainer()) {
        ss << crane.getHeldContainer()->getId() << "|";
    }

    // Include container positions in each stack
    for (size_t i = 0; i < stacks.size(); i++) {
        ss << "S" << i << ":";
        for (const auto& container : stacks[i]) {
            if (container.getExitTime() == -1) {
                ss << container.getId() << ",";
            }
        }
        ss << "|";
    }

    return ss.str();
}

bool AStarState::isGoalState() const {
    if (crane.getHeldContainer()) {
        return false;
    }
    for (size_t i = 0; i < stacks.size() - 1; i++) {
        for (const auto& container : stacks[i]) {
            if (container.getExitTime() == -1) {
                return false;
            }
        }
    }
    return true;
}

void AStarState::printState() const {
    std::cout << "\n╔══════════════════════════════════════════╗" << std::endl;
    std::cout << "║ State at time: " << std::setw(6) << current_time << " seconds          ║" << std::endl;
    std::cout << "╠══════════════════════════════════════════╣" << std::endl;

    double exitedLateness = getTotalLateness();
    
    std::cout << "║ Exited containers lateness: " << std::setw(6) << std::fixed 
              << std::setprecision(0) << exitedLateness << " sec    ║" << std::endl;
    std::cout << "╠══════════════════════════════════════════╣" << std::endl;

    std::cout << "║ " << crane.toString();

    int craneStrLen = crane.toString().length();
    for (int i = craneStrLen; i < 40; i++) {
        std::cout << " ";
    }
    std::cout << " ║" << std::endl;

    std::cout << "╠══════════════════════════════════════════╣" << std::endl;


    for (size_t i = 0; i < stacks.size(); i++) {
        std::cout << "║ Stack " << i;

        if (i == 0) {
            std::cout << " (Entry)    : ";
        } else if (i == stacks.size() - 1) {
            std::cout << " (Outgoing) : ";
        } else {
            std::cout << " (Buffer)   : ";
        }

        if (stacks[i].empty()) {
            std::cout << "(empty)";
        } else {
            for (size_t j = 0; j < stacks[i].size(); j++) {
                const auto& container = stacks[i][j];
                if (container.getExitTime() == -1 || container.getExitTime() != -1) {
                     std::cout << container.getId();
            
                    int absoluteDueTime = container.getArrivalTime() + container.getDueIn();
                    std::cout << "(due:" << absoluteDueTime << ")";
            
                    if (absoluteDueTime < current_time) {
                            std::cout << "[LATE by " << (current_time - absoluteDueTime) << "s]";
                    }
                    if(container.getExitTime() != -1){                   
                         std::cout<< "[Exittime: " << container.getExitTime() << "s]";}
                    
                    if (j < stacks[i].size() - 1) {
                        std::cout << " ";
                    }
                }
            }
        }

        std::cout << std::endl;
    }

    std::cout << "╠══════════════════════════════════════════╣" << std::endl;
    std::cout << "║ Last action: " << std::left << std::setw(27) << lastAction << " ║" << std::endl;
    std::cout << "║ Accumulated cost: " << std::fixed << std::setprecision(1)
              << std::setw(22) << accumulatedCost << " ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════╝" << std::endl;
}

int AStarState::getTotalContainers() const {
    int count = 0;
    for (const auto& stack : stacks) {
        count += stack.size();
    }
    return count;
}

int AStarState::getUnexitedContainers() const {
    int count = 0;
    for (const auto& stack : stacks) {
        for (const auto& container : stack) {
            if (container.getExitTime() == -1) {
                count++;
            }
        }
    }
    return count;
}

std::pair<int, int> AStarState::findContainer(const std::string& containerId) const {
    for (size_t stackIdx = 0; stackIdx < stacks.size(); stackIdx++) {
        for (size_t pos = 0; pos < stacks[stackIdx].size(); pos++) {
            if (stacks[stackIdx][pos].getId() == containerId) {
                return {stackIdx, pos};
            }
        }
    }
    return {-1, -1}; 
}

bool AStarState::canPickUpFrom(int stackIndex) const {
    if (stackIndex < 0 || stackIndex >= static_cast<int>(stacks.size())) {
        return false;
    }

    // Can't pick up from outgoing stack
    if (stackIndex == static_cast<int>(stacks.size()) - 1) {
        return false;
    }

    // Check if stack is empty
    if (stacks[stackIndex].empty()) {
        return false;
    }

    // Check if top container has already exited
    const auto& topContainer = stacks[stackIndex].back();
    if (topContainer.getExitTime() != -1) {
        return false;
    }

    // Crane must be empty
    if (crane.getHeldContainer()) {
        return false;
    }

    return true;
}

bool AStarState::canPutDownOn(int stackIndex, int bufferSize) const {

    if (stackIndex < 0 || stackIndex >= static_cast<int>(stacks.size())) {
        std::cout << "  -> Invalid stack index" << std::endl;
        return false;
    }

    if (stackIndex == 0) {
        return false;
    }

    if (!crane.hasContainer) {
        std::cout << "  -> Crane is not holding a container" << std::endl;
        return false;
    }

    if (stackIndex != static_cast<int>(stacks.size()) - 1) {
        if (static_cast<int>(stacks[stackIndex].size()) >= bufferSize) {
            std::cout << "  -> Stack is full (buffer size limit reached)" << std::endl;
            return false;
        }
    }

    if(crane.position == stackIndex){
        return false;
    }

    if (!stacks[stackIndex].empty()) {
    const auto& topContainer = stacks[stackIndex].back();
    const auto& heldContainer = crane.heldContainer.value();
    
    // Calculate due times for comparison
    int topContainerDueTime = topContainer.getArrivalTime() + topContainer.getDueIn();
    int heldContainerDueTime = heldContainer.getArrivalTime() + heldContainer.getDueIn();
    
    // For non-exit stacks: Don't allow placing if the top container is due sooner than the held container
    if (stackIndex != static_cast<int>(stacks.size()) - 1) {
        if (topContainerDueTime < heldContainerDueTime) {
            return false;
        }
    }
    
    if (stackIndex == static_cast<int>(stacks.size()) - 1) {
        if (topContainer.getExitTime() != -1) {
            int nextBoundary = ((current_time / 60) + 1) * 60;
            
            int topContainerNewExitTime = nextBoundary + 60;
            
            if (topContainerNewExitTime > topContainer.getExitTime() + 4000) {
                return false;
            }
            
        }
        if (topContainerDueTime < heldContainerDueTime) {
            return false;
        }
    }
}
    return true;
    }


double AStarState::getTotalLateness() const {
    return totalAccumulatedLateness;
}

void AStarState::setTotalLateness(int late) {
    totalAccumulatedLateness += late;
}

UntilDueContainer* AStarState::getTopContainer(int stackIndex) {
    if (stackIndex < 0 || stackIndex >= static_cast<int>(stacks.size())) {
        return nullptr;
    }

    if (stacks[stackIndex].empty()) {
        return nullptr;
    }

    return &stacks[stackIndex].back();
}

const UntilDueContainer* AStarState::getTopContainer(int stackIndex) const {
    if (stackIndex < 0 || stackIndex >= static_cast<int>(stacks.size())) {
        return nullptr;
    }

    if (stacks[stackIndex].empty()) {
        return nullptr;
    }

    return &stacks[stackIndex].back();
}

bool AStarState::operator==(const AStarState& other) const {
    if (current_time != other.current_time) {
        return false;
    }

    bool holdingA = (crane.getHeldContainer() != nullptr);
    bool holdingB = (other.crane.getHeldContainer() != nullptr);

    if (crane.position != other.crane.position || holdingA != holdingB) {
        return false;
    }

    if (holdingA && holdingB) {
        if (crane.getHeldContainer()->getId() != other.crane.getHeldContainer()->getId()) {
            return false;
        }
    }

    if (stacks.size() != other.stacks.size()) {
        return false;
    }

    for (size_t i = 0; i < stacks.size(); i++) {
        if (stacks[i].size() != other.stacks[i].size()) {
            return false;
        }

        for (size_t j = 0; j < stacks[i].size(); j++) {
            if (stacks[i][j].getExitTime() == -1 || other.stacks[i][j].getExitTime() == -1) {
                if (stacks[i][j].getId() != other.stacks[i][j].getId() ||
                    stacks[i][j].getDueIn() != other.stacks[i][j].getDueIn()) {
                    return false;
                }
            }
        }
    }

    return true;
}
