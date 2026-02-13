#include "AStarSolver.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <set>
#include <thread>
#include <chrono>


// NodeComparator implementation
bool NodeComparator::operator()(const std::shared_ptr<AStarNode>& a, 
                               const std::shared_ptr<AStarNode>& b) const {
    // For min-heap, return true if a has higher f-value than b
    return a->f > b->f;
}

AStarSolver::AStarSolver(const ParsedBuffers& buffers, int maxNodes, bool verbose, int maxSolutions) 
    : maxNodes(maxNodes), verbose(verbose), maxSolutionsToFind(maxSolutions), nodesExpanded(0),
      nodesGenerated(0), duplicatesDetected(0), searchElapsedTime(0.0) {
    
    heuristic = std::make_unique<LatenessHeuristic>(buffers);
    generator = std::make_unique<StateGenerator>(buffers);
}

AStarSolution AStarSolver::solve(const AStarState& initialState) {
    auto startTime = std::chrono::high_resolution_clock::now();
    initialContainerCount = initialState.getUnexitedContainers();
    AStarSolution solution;
    
    nodesExpanded = 0;
    nodesGenerated = 0;
    duplicatesDetected = 0;
    allSolutions.clear();  
    
    
    if (initialState.isGoalState()) {
        solution.found = true;
        solution.path.push_back(initialState);
        solution.totalCost = 0;
        return solution;
    }
    
    std::priority_queue<std::shared_ptr<AStarNode>, 
                       std::vector<std::shared_ptr<AStarNode>>, 
                       NodeComparator> openSet;
    
    // Closed set to track visited states
    std::unordered_set<std::string> closedSet;
    
    // Also keep track of best g-values for each state
    std::unordered_map<std::string, double> bestG;
    
    bool foundFirstSolution = false;
    double firstSolutionCost = std::numeric_limits<double>::max();
    
    // Create initial node
    double g0 = initialState.getTotalLateness();   
    double h0 = heuristic->evaluate(initialState); // estimated future lateness

    auto startNode = std::make_shared<AStarNode>(initialState, g0, h0);
    openSet.push(startNode);
    bestG[initialState.getStateHash()] = g0;
    nodesGenerated++;

    
    if (verbose) {
        std::cout << "\n=== A* Search Started ===" << std::endl;
        std::cout << "Initial heuristic value: " << h0 << std::endl;
        std::cout << "Max nodes limit: " << maxNodes << std::endl;
        std::cout << "Looking for up to " << maxSolutionsToFind << " solutions" << std::endl;
    }
    
    // Main A* loop
    while (!openSet.empty() && nodesExpanded < maxNodes) {
        auto current = openSet.top();
        openSet.pop();
        

        if (current->state.isGoalState()) {
            CompleteSolution completeSol;
            completeSol.path = reconstructPath(current);
            completeSol.totalCost = current->g;
            completeSol.totalLateness = current->state.getTotalLateness();
            completeSol.nodesExpandedWhenFound = nodesExpanded;
            
            for (size_t i = 0; i < std::min(completeSol.path.size(), size_t(5)); i++) {
                completeSol.keyMoves.push_back(completeSol.path[i].lastAction);
            }
            if (completeSol.path.size() > 7) {
                completeSol.keyMoves.push_back("...");
                for (size_t i = completeSol.path.size() - 2; i < completeSol.path.size(); i++) {
                    completeSol.keyMoves.push_back(completeSol.path[i].lastAction);
                }
            }
            
            allSolutions.push_back(completeSol);
            
            if (!foundFirstSolution) {
                foundFirstSolution = true;
                firstSolutionCost = current->g;
                
                if (verbose) {
                    std::cout << "\nFirst solution found! Cost: " << current->g 
                              << ", Lateness: " << completeSol.totalLateness
                              << ". Continuing search for alternatives..." << std::endl;
                }
            } else {
                if (verbose) {
                    std::cout << "\nAlternative solution #" << allSolutions.size() 
                              << " found! Cost: " << current->g 
                              << ", Lateness: " << completeSol.totalLateness << std::endl;
                }
            }
            
            if (allSolutions.size() >= maxSolutionsToFind) {
                if (verbose) {
                    std::cout << "Found " << maxSolutionsToFind << " solutions. Stopping search." << std::endl;
                }
                break;
            }
            
            continue;
        }
        
        std::string stateHash = current->state.getStateHash();
        if (closedSet.count(stateHash) > 0) {
            duplicatesDetected++;
            continue;
        }
        
        nodesExpanded++;

        closedSet.insert(stateHash);
        
        if (verbose && nodesExpanded % 100 == 0) {
            printSearchProgress(nodesExpanded, openSet.size(), current->f);
        }

        auto successors = generator->generateSuccessors(current->state);
        
        if (verbose) {
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            std::cout << "\n=== NODE " << nodesExpanded << " (" << current->state.lastAction 
                      << ") f=" << current->f << " ===" << std::endl;
            std::cout << "Time: " << current->state.current_time << "s | ";
            std::cout << "Crane: ";
            if (current->state.crane.hasContainer) {
                std::cout << "holding " << current->state.crane.containerId << " at stack " << current->state.crane.position;
            } else {
                std::cout << "empty at stack " << current->state.crane.position;
            }
            std::cout << std::endl;
            std::cout << "Stacks: ";
            for (size_t i = 0; i < current->state.stacks.size(); i++) {
                std::cout << "S" << i << "[";
                for (size_t j = 0; j < current->state.stacks[i].size(); j++) {
                    const auto& container = current->state.stacks[i][j];
                    std::cout << container.getId();
                    if (container.getExitTime() != -1) {
                        std::cout << "(exited)";
                    } else {
                        int dueTime = container.getArrivalTime() + container.getDueIn();
                        std::cout << "(due:" << dueTime << ")";
                    }
                    if (j < current->state.stacks[i].size() - 1) std::cout << ",";
                }
                std::cout << "] ";
            }
            std::cout << std::endl;
        }
        
        int successorIndex = 0;
        for (const auto& [nextState, actionCost] : successors) {
            successorIndex++;
            
            // Set g to total lateness of the successor state (not accumulated cost)
            double g = calculateSimpleCost(nextState, current);

            std::string nextHash = nextState.getStateHash();

            auto it = bestG.find(nextHash);
            if (it != bestG.end() && it->second <= g) {
                duplicatesDetected++;
                if (verbose) {
                    std::cout << "  " << successorIndex << ". " << nextState.lastAction 
                              << " → DUPLICATE (skipped)" << std::endl;
                }
                continue;
            }

            bestG[nextHash] = g;

            double h = heuristic->evaluate(nextState);
            double f = g + h;

            if (verbose) {
                std::cout << "  " << successorIndex << ". " << nextState.lastAction 
                          << " → g=" << g << ", h=" << h << ", f=" << f;
                if (nextState.crane.hasContainer) {
                    std::cout << " (holding " << nextState.crane.containerId << ")";
                }
                std::cout << std::endl;
            }

            auto nextNode = std::make_shared<AStarNode>(nextState, g, h, current);
            openSet.push(nextNode);
            nodesGenerated++;

            if (verbose && nodesGenerated % 500 == 0) {
                std::cout << "  Generated " << nodesGenerated << " nodes..." << std::endl;
            }
        }
    }
    
    std::sort(allSolutions.begin(), allSolutions.end(),
        [](const CompleteSolution& a, const CompleteSolution& b) {
            if (std::abs(a.totalCost - b.totalCost) < 0.01) {
                return a.totalLateness < b.totalLateness;
            }
            return a.totalCost < b.totalCost;
        });
    
    // Return the best solution 
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime - startTime;
    searchElapsedTime = elapsed.count();
    
    if (!allSolutions.empty()) {
        solution.found = true;
            // Save best solution moves to separate file
        std::ofstream movesFile("BestSolutionMoves.txt");
        for (size_t i = 1; i < allSolutions[0].path.size(); ++i) {
            if (i > 1) movesFile << ";;";
            movesFile << allSolutions[0].path[i].lastAction;
        }
        movesFile.close();
        solution.path = allSolutions[0].path;
        solution.totalCost = allSolutions[0].totalCost;
        solution.nodesExpanded = nodesExpanded;
        solution.nodesGenerated = nodesGenerated;
        solution.searchElapsedTime = elapsed.count();
    } else {
        solution.found = false;
        solution.nodesExpanded = nodesExpanded;
        solution.nodesGenerated = nodesGenerated;
        solution.searchElapsedTime = elapsed.count();
        
        if (verbose) {
            std::cout << "\n=== Search Failed ===" << std::endl;
            if (nodesExpanded >= maxNodes) {
                std::cout << "Reached maximum node limit!" << std::endl;
            } else {
                std::cout << "No solution exists!" << std::endl;
            }
        }
    }
    
    return solution;
}

std::vector<AStarState> AStarSolver::reconstructPath(std::shared_ptr<AStarNode> goalNode) const {
    std::vector<AStarState> path;
    
    auto current = goalNode;
    while (current != nullptr) {
        path.push_back(current->state);
        current = current->parent;
    }
    
    std::reverse(path.begin(), path.end());
    return path;
}

void AStarSolver::printSearchProgress(int expanded, int queueSize, double bestF) const {
    std::cout << "Progress: Expanded=" << std::setw(6) << expanded 
              << ", Queue=" << std::setw(6) << queueSize 
              << ", Best f=" << std::fixed << std::setprecision(2) << bestF 
              << std::endl;
}

void AStarSolver::printStatistics() const {
    std::cout << "\n=== A* Search Statistics ===" << std::endl;
    std::cout << "Nodes expanded: " << nodesExpanded << std::endl;
    std::cout << "Nodes generated: " << nodesGenerated << std::endl;
    std::cout << "Duplicates detected: " << duplicatesDetected << std::endl;
    std::cout << "Solutions found: " << allSolutions.size() << std::endl; 
    std::cout << "Solutions time: " << searchElapsedTime << std::endl;
    double branchingFactor = nodesExpanded > 0 ? 
        static_cast<double>(nodesGenerated) / nodesExpanded : 0;
    std::cout << "Effective branching factor: " << std::fixed 
              << std::setprecision(2) << branchingFactor << std::endl;
}

double AStarSolver::calculateSimpleCost(const AStarState& state, 
                                       std::shared_ptr<AStarNode> parentNode) const {
        // Primary cost: actual lateness
    double cost = state.getTotalLateness();
    
    // Tiny tie-breaker: prefer fewer moves 
    cost += state.current_time * 0.001;
    
    static int debugCount = 0;
    if (debugCount < 10 && verbose) {
        debugSimpleCost(state, parentNode);
        debugCount++;
    }
    
    return cost;
}

int AStarSolver::calculateMoveCount(const AStarState& state, 
                                   std::shared_ptr<AStarNode> parentNode) const {
    if (!parentNode) {
        return 0; 
    }
    
    // Count moves by tracing back through parent chain
    int moveCount = 0;
    auto current = parentNode;
    
    // Trace back to root, counting each step
    while (current != nullptr && current->parent != nullptr) {
        moveCount++;
        current = current->parent;
    }
    
    // Add 1 for the current move (from parent to this state)
    if (state.lastAction != "Initial state" && state.lastAction != "") {
        moveCount++;
    }
    
    return moveCount;
}

int AStarSolver::calculateIdlePeriods(const AStarState& state) const {
    if (state.current_time == 0) {
        return 0;  
    }
    
    int totalPeriods = state.current_time / 60;
    
    // Calculate how many containers have been processed
    int processedContainers = initialContainerCount - state.getUnexitedContainers();
    
    int idlePeriods = std::max(0, totalPeriods - processedContainers);
    
    return idlePeriods;
}

void AStarSolver::debugSimpleCost(const AStarState& state, 
                                 std::shared_ptr<AStarNode> parentNode) const {
    double lateness = state.getTotalLateness();
    int moves = calculateMoveCount(state, parentNode);
    int idle = calculateIdlePeriods(state);
    int unexited = state.getUnexitedContainers();
    
    double latenessCost = lateness * 10.0;
    double idleCost = idle * 100.0;
    double unexitedCost = unexited * 120.0;
    double totalCost = latenessCost + idleCost + unexitedCost;
    
    std::cout << "\n=== COST BREAKDOWN: " << state.lastAction << " ===" << std::endl;
    std::cout << "Time: " << state.current_time << "s" << std::endl;
    std::cout << "Lateness:      " << lateness << " * 10 = " << latenessCost << std::endl;
    std::cout << "Idle periods:  " << idle << " * 100 = " << idleCost << std::endl;
    std::cout << "Unexited:      " << unexited << " * 50 = " << unexitedCost << std::endl;
    std::cout << "TOTAL COST:    " << totalCost << std::endl;
    
    if (idle > 0) {
        std::cout << "  (Idle details: " << (state.current_time / 60) 
                  << " total periods, " << idle << " without exits)" << std::endl;
    }
    
    std::cout << "===========================================" << std::endl;
}