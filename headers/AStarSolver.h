#ifndef ASTAR_SOLVER_H
#define ASTAR_SOLVER_H

#include "AStarState.h"
#include "LatenessHeuristic.h"
#include "StateGenerator.h"
#include "ParsedBuffers.h"
#include <vector>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <chrono>

struct AStarNode;

struct NodeComparator {
    bool operator()(const std::shared_ptr<AStarNode>& a, 
                   const std::shared_ptr<AStarNode>& b) const;
};

struct AStarNode {
    AStarState state;
    double g;      double h;      double f;      std::shared_ptr<AStarNode> parent;      
    AStarNode(const AStarState& s, double gCost, double hCost, 
              std::shared_ptr<AStarNode> p = nullptr)
        : state(s), g(gCost), h(hCost), f(gCost + hCost), parent(p) {}
};

struct AStarSolution {
    bool found;
    std::vector<AStarState> path;
    double totalCost;
    int nodesExpanded;
    int nodesGenerated;
    double searchElapsedTime;      
    AStarSolution() : found(false), totalCost(0), nodesExpanded(0), 
                     nodesGenerated(0), searchElapsedTime(0) {}
};

struct CompleteSolution {
    std::vector<AStarState> path;
    double totalCost;
    double totalLateness;
    int nodesExpandedWhenFound;
    std::vector<std::string> keyMoves;  };

class AStarSolver {
private:
    std::unique_ptr<LatenessHeuristic> heuristic;
    std::unique_ptr<StateGenerator> generator;
    
        int maxNodes;      bool verbose;      int maxSolutionsToFind;      int initialContainerCount;
    
        mutable int nodesExpanded;
    mutable int nodesGenerated;
    mutable int duplicatesDetected;
    mutable double searchElapsedTime;      
        std::vector<CompleteSolution> allSolutions;
    
    double calculateSimpleCost(const AStarState& state, 
                              std::shared_ptr<AStarNode> parentNode) const;
    int calculateMoveCount(const AStarState& state, 
                          std::shared_ptr<AStarNode> parentNode) const;
    int calculateIdlePeriods(const AStarState& state) const;
    void debugSimpleCost(const AStarState& state, 
                        std::shared_ptr<AStarNode> parentNode) const;

        std::vector<AStarState> reconstructPath(std::shared_ptr<AStarNode> goalNode) const;
    void printSearchProgress(int expanded, int queueSize, double bestF) const;
    
public:
        explicit AStarSolver(const ParsedBuffers& buffers, int maxNodes = 100000, 
                        bool verbose = false, int maxSolutions = 10);
    
        AStarSolution solve(const AStarState& initialState);
    
        const std::vector<CompleteSolution>& getAllSolutions() const { 
        return allSolutions; 
    }
    
        void printStatistics() const;
    int getNodesExpanded() const { return nodesExpanded; }
    int getNodesGenerated() const { return nodesGenerated; }
    int getDuplicatesDetected() const { return duplicatesDetected; }
    
        void setVerbose(bool v) { verbose = v; }
    void setMaxNodes(int max) { maxNodes = max; }
};

#endif 