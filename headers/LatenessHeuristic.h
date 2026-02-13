#ifndef LATENESS_HEURISTIC_H
#define LATENESS_HEURISTIC_H

#include "IHeuristic.h"
#include "ParsedBuffers.h"
#include <string>

class LatenessHeuristic : public IHeuristic {
private:
        int craneMoveTime;       int craneLowerTime;      int craneLiftTime;       int clearingTime;        
        double calculateMinimumLateness(const AStarState& state) const;
    int calculateMinTimeToExit(const AStarState& state, 
                              int stackIndex, 
                              int containerPosition) const;
    int getMinMovesBetweenStacks(int from, int to) const;
    
public:
        explicit LatenessHeuristic(const ParsedBuffers& buffers);
    
        double evaluate(const AStarState& state) const override;
    std::string getName() const override { return "Lateness Heuristic"; }
    
        int getCraneMoveTime() const { return craneMoveTime; }
    int getCraneLowerTime() const { return craneLowerTime; }
    int getCraneLiftTime() const { return craneLiftTime; }
    int getClearingTime() const { return clearingTime; }
};

#endif 