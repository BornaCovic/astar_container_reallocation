#ifndef STATE_GENERATOR_H
#define STATE_GENERATOR_H

#include "AStarState.h"
#include "ParsedBuffers.h"
#include <vector>
#include <memory>

struct Action {
    enum Type { PICK_UP, PUT_DOWN, WAIT };
    Type type;
    int targetStack;
    std::string description;
    int waitTime;      
    Action(Type t, int stack, const std::string& desc, int wait = 0) 
        : type(t), targetStack(stack), description(desc), waitTime(wait) {}
};

class StateGenerator {
public:
    StateGenerator(const ParsedBuffers& buffers);
    
        std::vector<std::pair<AStarState, double>> generateSuccessors(const AStarState& current) const;
    
        std::vector<Action> getValidActions(const AStarState& current) const;

private:
    const ParsedBuffers& buffers;
    int craneMoveTime;
    int craneLowerTime;
    int craneLiftTime;
    int clearingTime;
    
        AStarState applyPickUp(const AStarState& current, int stackIndex, double& cost) const;
    AStarState applyPutDown(const AStarState& current, int stackIndex, double& cost) const;
    AStarState applyWait(const AStarState& current, int waitTime, double& cost) const;
    
        void updateAllContainerTimes(AStarState& state, int elapsedTime) const;
    int calculateCraneMoveTime(int from, int to) const;
    void clearExitedContainers(AStarState& state, int elapsedTime) const;
    
        bool shouldConsiderWaiting(const AStarState& current) const;
    bool canWaitingHelp(const AStarState& current) const;
    bool hasWaitedTooMuch(const AStarState& current) const;
    int calculateOptimalWaitTime(const AStarState& current) const;
    
        static constexpr int MAX_CONSECUTIVE_WAITS = 6;
    static constexpr double MAX_WAIT_RATIO = 1.0;      static constexpr int MAX_WAIT_TIME = 10;       };

#endif 