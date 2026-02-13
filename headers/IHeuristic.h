#ifndef IHEURISTIC_H
#define IHEURISTIC_H

#include "AStarState.h"
#include <string>

// Base interface for all heuristics
class IHeuristic {
public:
    virtual ~IHeuristic() = default;
    
    // Evaluate the heuristic value for a given state
    // Returns estimated cost from this state to goal
    virtual double evaluate(const AStarState& state) const = 0;
    
    // Get name of the heuristic for debugging
    virtual std::string getName() const = 0;
};

#endif // IHEURISTIC_H