#include <unistd.h>
#include <thread>
#include <fstream>
#include <vector>
#include <sstream>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "HotStorageSimulator.h"
#include "EntryContainerStack.h"
#include "OutGoingContainerStack.h"
#include "AStarSolver.h"
#include "AStarStartingState.h"


HotStorageSimulator::HotStorageSimulator(Printer &p) : printer(&p) {
    needsRecalculation = false;
    systemPaused = false;
    systemTime = 0;
}

std::vector<std::string> HotStorageSimulator::loadBestSolutionMoves() {
    std::vector<std::string> moves;
    std::ifstream file("BestSolutionMoves.txt");
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open BestSolutionMoves.txt" << std::endl;
        return moves;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
        size_t start = 0;
    size_t end = content.find(";;");
    
    while (end != std::string::npos) {
        std::string move = content.substr(start, end - start);
        if (!move.empty()) {
            moves.push_back(move);
        }
        start = end + 2;
        end = content.find(";;", start);
    }
    
    if (start < content.length()) {
        std::string lastMove = content.substr(start);
        if (!lastMove.empty()) {
            moves.push_back(lastMove);
        }
    }
    
    return moves;
}

std::vector<std::string> HotStorageSimulator::recalculateAStar() {
    auto data = printer->getParsedBuffers();
    SingleContainerCrane *crane = dynamic_cast<SingleContainerCrane*>(printer->getCrane());
    
    std::cout << "\n=== DEBUGGING A* SEARCH FAILURE ===" << std::endl;
    
        AStarState currentState = makeAStarCurrentState(*data, systemTime.load(), crane);
    
        std::cout << "\n--- INITIAL STATE ANALYSIS ---" << std::endl;
    currentState.printState();
    
        std::cout << "\n--- GOAL STATE CHECK ---" << std::endl;
    bool isGoal = currentState.isGoalState();
    std::cout << "Is goal state: " << (isGoal ? "YES" : "NO") << std::endl;
    
    if (!isGoal) {
        std::cout << "Why not goal state:" << std::endl;
        
                if (currentState.crane.getHeldContainer()) {
            std::cout << "  - Crane is holding container: " 
                      << currentState.crane.getHeldContainer()->getId() << std::endl;
        } else {
            std::cout << "  - Crane is empty ✓" << std::endl;
        }
        
                for (size_t i = 0; i < currentState.stacks.size() - 1; i++) {
            std::cout << "  - Stack " << i << ": ";
            int unexited = 0;
            for (const auto& container : currentState.stacks[i]) {
                if (container.getExitTime() == -1) {
                    unexited++;
                    std::cout << container.getId() << "(unexited) ";
                }
            }
            if (unexited == 0) {
                std::cout << "all exited ✓";
            }
            std::cout << std::endl;
        }
    }
    
        std::cout << "\n--- SUCCESSOR GENERATION TEST ---" << std::endl;
    StateGenerator generator(*data);
    auto successors = generator.generateSuccessors(currentState);
    
    std::cout << "Generated " << successors.size() << " successor states:" << std::endl;
    
    if (successors.empty()) {
        std::cout << "*** CRITICAL: No successors generated! This explains why A* fails. ***" << std::endl;
        
                std::cout << "\nDebugging successor generation:" << std::endl;
        
                for (int i = 0; i < static_cast<int>(currentState.stacks.size()); i++) {
            bool canPickUp = currentState.canPickUpFrom(i);
            std::cout << "  Can pick up from stack " << i << ": " << (canPickUp ? "YES" : "NO");
            if (!canPickUp) {
                if (i == static_cast<int>(currentState.stacks.size()) - 1) {
                    std::cout << " (outgoing stack - not allowed)";
                } else if (currentState.stacks[i].empty()) {
                    std::cout << " (stack empty)";
                } else if (currentState.crane.getHeldContainer()) {
                    std::cout << " (crane not empty)";
                } else {
                    const auto& topContainer = currentState.stacks[i].back();
                    if (topContainer.getExitTime() != -1) {
                        std::cout << " (top container already exited)";
                    }
                }
            }
            std::cout << std::endl;
        }
        
                if (currentState.crane.getHeldContainer()) {
            for (int i = 1; i < static_cast<int>(currentState.stacks.size()); i++) {
                bool canPutDown = currentState.canPutDownOn(i, 5);                 std::cout << "  Can put down on stack " << i << ": " << (canPutDown ? "YES" : "NO") << std::endl;
            }
        }
        
                std::cout << "  Can wait: Should always be possible" << std::endl;
        
    } else {
                for (size_t i = 0; i < std::min(successors.size(), size_t(5)); i++) {
            const auto& [nextState, cost] = successors[i];
            std::cout << "  " << (i+1) << ". " << nextState.lastAction 
                      << " (time: " << nextState.current_time << "s)" << std::endl;
        }
        if (successors.size() > 5) {
            std::cout << "  ... and " << (successors.size() - 5) << " more" << std::endl;
        }
    }
    
        std::cout << "\n--- HEURISTIC TEST ---" << std::endl;
    LatenessHeuristic heuristic(*data);
    double hValue = heuristic.evaluate(currentState);
    std::cout << "Heuristic value: " << hValue << std::endl;
    
    if (hValue == std::numeric_limits<double>::infinity()) {
        std::cout << "*** CRITICAL: Heuristic returned infinity! ***" << std::endl;
    } else if (hValue < 0) {
        std::cout << "*** WARNING: Negative heuristic value ***" << std::endl;
    }
    
        std::cout << "\n--- MANUAL SOLUTION TEST ---" << std::endl;
    std::cout << "Expected simple solution:" << std::endl;
    std::cout << "1. Wait until time 60+ for B90 to clear" << std::endl;
    std::cout << "2. Wait until time 120+ for B91 to clear" << std::endl;
    std::cout << "3. Move B84 from entry to outgoing" << std::endl;
    std::cout << "4. Wait for B84 to clear" << std::endl;
    
        std::cout << "\n--- TESTING WAIT ACTION ---" << std::endl;
    AStarState waitState = currentState;
    waitState.current_time += 10;     waitState.lastAction = "Waited for 10 seconds";
    
        for (size_t i = 0; i < waitState.stacks.size(); i++) {
        for (auto& container : waitState.stacks[i]) {
            if (container.getExitTime() == -1 && i == waitState.stacks.size() - 1) {
                                int dueTime = container.getArrivalTime() + container.getDueIn();
                if (waitState.current_time >= dueTime + 60) {                     std::cout << "Container " << container.getId() 
                              << " should clear at time " << waitState.current_time << std::endl;
                }
            }
        }
    }
    
        std::cout << "\n--- RUNNING A* WITH DEBUGGING ---" << std::endl;
    AStarSolver solver(*data, 1000000, false, 1);     AStarSolution solution = solver.solve(currentState);
    
    std::cout << "\n--- A* RESULTS ---" << std::endl;
    std::cout << "Solution found: " << (solution.found ? "YES" : "NO") << std::endl;
    std::cout << "Nodes expanded: " << solution.nodesExpanded << std::endl;
    std::cout << "Nodes generated: " << solution.nodesGenerated << std::endl;
    
    if (!solution.found && solution.nodesExpanded == 0) {
        std::cout << "*** CRITICAL: A* didn't expand any nodes! ***" << std::endl;
        std::cout << "This suggests the initial state itself has issues." << std::endl;
    }
    
    solver.printStatistics();
    
    std::vector<std::string> transformedMoves;

        if (solution.found) {
                std::vector<std::string> bestMoves = loadBestSolutionMoves();
        std::cout << "Loaded " << bestMoves.size() << " raw moves from solution." << std::endl;
        
                for (size_t i = 0; i < bestMoves.size(); i++) {
            if (bestMoves[i].find("Picked up") != std::string::npos && 
                i + 1 < bestMoves.size() && 
                bestMoves[i + 1].find("Put down") != std::string::npos) {
                
                size_t fromPos = bestMoves[i].find("from stack ");
                int sourceStack = bestMoves[i][fromPos + 11] - '0';
                
                size_t onPos = bestMoves[i + 1].find("on stack ");
                int destStack = bestMoves[i + 1][onPos + 9] - '0';
                
                transformedMoves.push_back(std::to_string(sourceStack) + " " + std::to_string(destStack));
                i++;             }
            else if (bestMoves[i].find("Waited for 10 seconds") != std::string::npos) {
                int waitCount = 1;
                while (i + waitCount < bestMoves.size() && 
                       bestMoves[i + waitCount].find("Waited for 10 seconds") != std::string::npos) {
                    waitCount++;
                }
                transformedMoves.push_back("101010 " + std::to_string(waitCount));
                i += waitCount - 1;
            }
        }
        }
    
    return transformedMoves;
}

void HotStorageSimulator::runEntryStack(){
    auto data = printer->getParsedBuffers();
    EntryContainerStack *entryStack = dynamic_cast<EntryContainerStack*>(data->getBuffers().at(0));
    
    size_t lastKnownSize = entryStack->getSize();
    
    while (1) {
                {
            std::unique_lock<std::mutex> lock(recalcMutex);
            recalcCV.wait(lock, [this] { return !systemPaused; });
        }
        
                entryStack->startAutoAddContainers(1, 35);
        
                size_t currentSize = entryStack->getSize();
        if (currentSize > lastKnownSize) {
            std::cout << "\n*** NEW CONTAINER DETECTED ON ENTRY STACK ***" << std::endl;
            std::cout << "Size changed from " << lastKnownSize << " to " << currentSize << std::endl;
            std::cout << "Added " << (currentSize - lastKnownSize) << " new container(s)" << std::endl;
            
            needsRecalculation = true;
            lastKnownSize = currentSize;
        }
        
                sleep(1);     }
}

void HotStorageSimulator::runCrane(){
    printer->printEverything();

    auto data = printer->getParsedBuffers();
    SingleContainerCrane *crane = dynamic_cast<SingleContainerCrane*>(printer->getCrane());
    EntryContainerStack *entryStack = dynamic_cast<EntryContainerStack*>(data->getBuffers().at(0));
    OutGoingContainerStack *outgoingStack = dynamic_cast<OutGoingContainerStack*>(data->getBuffers().at(4));

    UntilDue lower = data->getCraneLower();
    UntilDue move = data->getCraneMove();
    UntilDue lift = data->getCraneLift();
    
    size_t moveIndex = 0;

    while(1){
                bool needsRecalc = false;
        size_t movesSize = 0;
        {
            std::lock_guard<std::mutex> lock(movesMutex);
            movesSize = currentMoves.size();
        }
        
        if (needsRecalculation && moveIndex < movesSize && crane->getHookContent() == nullptr) {
            std::cout << "\n*** PAUSING SYSTEM FOR A* RECALCULATION ***" << std::endl;
            
                        systemPaused = true;
            entryStack->pauseTime();
            outgoingStack->pauseTime();
            
                        std::vector<std::string> newMoves = recalculateAStar();
            
                        
            std::lock_guard<std::mutex> lock(movesMutex);
            currentMoves = newMoves;
            moveIndex = 0;             
            
                        systemPaused = false;
            needsRecalculation = false;
            entryStack->continueTime();
            outgoingStack->continueTime();
            recalcCV.notify_all();
            
            std::cout << "*** SYSTEM RESUMED WITH NEW PLAN ***" << std::endl;
            std::cout << "*** A ***" << std::endl;
        }
        
        int input1, input2;
        
                {
            std::lock_guard<std::mutex> lock(movesMutex);
            if (moveIndex < currentMoves.size()) {
                std::istringstream iss(currentMoves[moveIndex]);
                if (!(iss >> input1 >> input2)) {
                    std::cerr << "Malformed move: '" << currentMoves[moveIndex] << "', exiting runCrane\n";
                    return;
                }
                std::cout << "[Automated] Move " << (moveIndex + 1) << "/" << currentMoves.size() 
                          << ": " << input1 << " -> " << input2 << std::endl;
                ++moveIndex;
            } else {
                if (needsRecalculation) {
                std::cout << "\n*** PAUSING SYSTEM FOR A* RECALCULATION ***" << std::endl;
                
                                systemPaused = true;
                entryStack->pauseTime();
                outgoingStack->pauseTime();
                
                                std::vector<std::string> newMoves = recalculateAStar();
                
                                {
                    std::lock_guard<std::mutex> lock(movesMutex);
                    currentMoves = newMoves;
                    moveIndex = 0;                 }
                
                                systemPaused = false;
                needsRecalculation = false;
                entryStack->continueTime();
                outgoingStack->continueTime();
                recalcCV.notify_all();
                
                std::cout << "*** SYSTEM RESUMED WITH NEW PLAN ***" << std::endl;
            }
            }
        }

        if(input1 == input2){
            std::cout<<"Ne mozete premjestiti kontejner na isti stog!"<<std::endl;
            continue;
        }else if(input2 == 0){
            std::cout<<"Ne mozete stavljati kontejnere na dolazni stog!"<<std::endl;
            continue;
        }else if(input1 == 4){
            std::cout<<"Ne mozete uzimati kontejnere sa odlaznog stoga!"<<std::endl;
            continue;
        } else if(input1 == 101010){
            int waitSeconds = input2 * 10;
            data->refreshTime(UntilDue(0, waitSeconds));
            systemTime += waitSeconds;              std::cout<<"Spavam " << waitSeconds << " sekundi!"<<std::endl;
            sleep(waitSeconds);
            printer->printEverything();
            continue;
        }

        entryStack->continueTime();
        outgoingStack->continueTime();
        
                
        if(input1 != crane->getAboveStackIndex() && input1 != 101010){
            data->refreshTime(move);
            int moveTime = move.getMinutes() * 60 + move.getSeconds();
            systemTime += moveTime;              sleep(moveTime);
            crane->setAboveStackIndex(input1);
            std::cout<<"Kuka je pomaknuta iznad stoga "<< crane->getAboveStack() <<std::endl;
            printer->printEverything();
        }

        data->refreshTime(lower);
        int lowerTime = lower.getMinutes() * 60 + lower.getSeconds();
        systemTime += lowerTime;          std::cout<<"Kuka se spusta."<<std::endl;
        sleep(lowerTime);
        std::cout<<"Kuka se spustila, skupila kontejner i pocela se dizat."<<std::endl;

        UntilDueContainer *container = static_cast<UntilDueContainer*>(data->getBuffers().at(input1)->pop());
        crane->setHookContent(container);

        data->refreshTime(lift);
        int liftTime = lift.getMinutes() * 60 + lift.getSeconds();
        systemTime += liftTime;          sleep(liftTime);
        crane->refreshTime(lift);

        std::cout<<"Podignut je kontejner "<< container->getDetails()<<std::endl;

        printer->printEverything();

        crane->setAboveStackIndex(input2);

        data->refreshTime(move);
        crane->refreshTime(move);
        int moveTime2 = move.getMinutes() * 60 + move.getSeconds();
        systemTime += moveTime2;          sleep(moveTime2);
        std::cout<<"Kuka je pomaknuta iznad stoga "<< crane->getAboveStack() <<std::endl;
        printer->printEverything();

        data->refreshTime(lower);
        crane->refreshTime(lower);
        int lowerTime2 = lower.getMinutes() * 60 + lower.getSeconds();
        systemTime += lowerTime2;          std::cout<<"Kuka se spusta."<<std::endl;
        sleep(lowerTime2);

        if(input2 != 4)
            data->getBuffers().at(input2)->push(*container);
        else
            data->getBuffers().at(input2)->push_infront(*container);

        std::cout<<"Kuka je ostavila kontejner na stog i pocela se dizat."<<std::endl;
        crane->setHookContent(nullptr);
        printer->printEverything();

        data->refreshTime(lift);
        int liftTime2 = lift.getMinutes() * 60 + lift.getSeconds();
        systemTime += liftTime2;          sleep(liftTime2);
        std::cout<<"Kuka spremna za iducu naredbu —>"<<std::endl;
        printer->printEverything();

        entryStack->pauseTime();
        outgoingStack->pauseTime();
        
            }
}

void HotStorageSimulator::runOutgoingStack(){
    auto data = printer->getParsedBuffers();
    OutGoingContainerStack *outgoingStack = dynamic_cast<OutGoingContainerStack*>(data->getBuffers().at(4));

    while (1) {
                {
            std::unique_lock<std::mutex> lock(recalcMutex);
            recalcCV.wait(lock, [this] { return !systemPaused; });
        }
        
        outgoingStack->startPoppingContainers(data->getClearingTime().getSeconds() + data->getClearingTime().getMinutes() * 60);
    }
}

void HotStorageSimulator::simulate(){
    auto data = printer->getParsedBuffers();
    
        EntryContainerStack *entryStack = dynamic_cast<EntryContainerStack*>(data->getBuffers().at(0));
    entryStack->setOnContainerAddedCallback([this]() {
        std::cout << "\n*** NEW CONTAINER DETECTED ON ENTRY STACK ***" << std::endl;
        needsRecalculation = true;
    });

        std::cout << "Running initial A* algorithm..." << std::endl;
    
    AStarState initialState = makeAStarInitialState(*data);
    AStarSolver solver(*data, 1000000, false, 1);
    AStarSolution solution = solver.solve(initialState);
    
    if (solution.found) {
        std::cout << "A* found a solution!" << std::endl;
        
        std::vector<std::string> bestMoves = loadBestSolutionMoves();
        std::cout << "Loaded " << bestMoves.size() << " moves from A* solution." << std::endl;
        
                for (size_t i = 0; i < bestMoves.size(); i++) {
            if (bestMoves[i].find("Picked up") != std::string::npos && 
                i + 1 < bestMoves.size() && 
                bestMoves[i + 1].find("Put down") != std::string::npos) {
                
                size_t fromPos = bestMoves[i].find("from stack ");
                int sourceStack = bestMoves[i][fromPos + 11] - '0';
                
                size_t onPos = bestMoves[i + 1].find("on stack ");
                int destStack = bestMoves[i + 1][onPos + 9] - '0';
                
                currentMoves.push_back(std::to_string(sourceStack) + " " + std::to_string(destStack));
                i++;
            }
            else if (bestMoves[i].find("Waited for 10 seconds") != std::string::npos) {
                int waitCount = 1;
                while (i + waitCount < bestMoves.size() && 
                       bestMoves[i + waitCount].find("Waited for 10 seconds") != std::string::npos) {
                    waitCount++;
                }
                currentMoves.push_back("101010 " + std::to_string(waitCount));
                i += waitCount - 1;
            }
        }
        
        std::cout << "\nInitial transformed moves:" << std::endl;
        for (const auto& move : currentMoves) {
            std::cout << move << std::endl;
        }
    } else {
        std::cout << "A* could not find a solution!" << std::endl;
    }
    
    std::cout << "\nStarting simulation with dynamic A* recalculation..." << std::endl;

        std::thread entryStackThread(&HotStorageSimulator::runEntryStack, this);
    std::thread craneThread(&HotStorageSimulator::runCrane, this);
    std::thread outgoingStackThread(&HotStorageSimulator::runOutgoingStack, this);

        entryStackThread.join();
    craneThread.join();
    outgoingStackThread.join();
}