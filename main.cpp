#include <iostream>
#include <string>
#include <algorithm>
#include <exception>
#include <limits>
#include <fstream>
#include <thread>
#include <chrono>


#include "ParsedBuffers.h"
#include "AStarState.h"
#include "AStarSolver.h"
#include "AStarStartingState.h"

// Forward declarations of printing functions
void printAllSolutions(const AStarSolver& solver) {
    auto solutions = solver.getAllSolutions();

    if (solutions.empty()) {
        std::cout << "\nNo solutions found!" << std::endl;
        return;
    }

    std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║              ALL SOLUTIONS FOUND                   ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;
    std::cout << "\nTotal solutions found: " << solutions.size() << std::endl;

    // (rest of your printAllSolutions code exactly as given)
    // ...
}

void printDetailedSolution(const CompleteSolution& solution, int solutionNumber, bool isBest = false) {
    if (isBest) {
        std::cout << "\n╔════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║           BEST SOLUTION DETAILS                ║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════╝" << std::endl;
    } else {
        std::cout << "\n╔════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║        ALTERNATIVE SOLUTION #" << solutionNumber << " DETAILS        ║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════╝" << std::endl;
    }

    for (size_t i = 0; i < solution.path.size(); i++) {
        std::cout << "\nStep " << i << ":" << std::endl;
        solution.path[i].printState();
    }

    std::cout << "\nFinal Statistics:" << std::endl;
    std::cout << "Total Cost: " << solution.totalCost << " seconds" << std::endl;
    std::cout << "Total Lateness: " << solution.totalLateness << " seconds" << std::endl;
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <config_file> [verbose]" << std::endl;
        return 1;
    }

    bool verbose = (argc > 2 && std::string(argv[2]) == "verbose");

    // Open file for saving results
    std::ofstream MyFile("AStarProcess.txt");
    std::streambuf* coutbuf = std::cout.rdbuf(); // save original console buffer

    try {
        // === PART 1: SETUP - SAVE TO FILE ===
        std::cout.rdbuf(MyFile.rdbuf()); // redirect to file
        
        // Load configuration
        std::cout << "Loading configuration from: " << argv[1] << std::endl;
        ParsedBuffers buffers(argv[1]);

        // Create initial state
        std::cout << "Creating initial state..." << std::endl;
        AStarState initialState = makeAStarInitialState(buffers);

        // Create solver that finds up to 10 solutions
        AStarSolver solver(buffers, 1000000000, verbose, 10);

        std::cout << "\nRunning A* search for multiple solutions..." << std::endl;
        
        // === PART 2: A* SEARCH - SHOW ON CONSOLE IF VERBOSE ===
        if (verbose) {
            std::cout.rdbuf(coutbuf); // restore console for verbose debug output
            std::cout << "\n*** SHOWING A* DEBUG OUTPUT ON CONSOLE ***" << std::endl;
            std::cout << "*** (Results will still be saved to file) ***" << std::endl;
        }
        
        // Solve (debug output will show on console if verbose)
        AStarSolution bestSolution = solver.solve(initialState);
        
        if (verbose) {
            std::cout << "\n*** A* SEARCH COMPLETED - SWITCHING BACK TO FILE ***" << std::endl;
            std::cout.rdbuf(MyFile.rdbuf()); // redirect back to file
        }

        // === PART 3: RESULTS - SAVE TO FILE ===
        if (bestSolution.found) {
            // Get all solutions (CompleteSolution)
            std::vector<CompleteSolution> allSolutions = solver.getAllSolutions();

            // Print best solution details (the first in allSolutions)
            if (!allSolutions.empty()) {
                std::ofstream movesFile("BestSolutionMoves.txt");
                for (size_t i = 1; i < allSolutions[0].path.size(); ++i) {
                    if (i > 1) movesFile << ";;";
                    movesFile << allSolutions[0].path[i].lastAction;
                    }
                movesFile.close();
    
                printDetailedSolution(allSolutions[0], 0, true);
            }

            // Print summary comparison of all solutions
            printAllSolutions(solver);

            // Ask user if they want to see detailed alternative solutions
            if (allSolutions.size() > 1) {
                // Temporarily switch to console for user interaction
                std::cout.rdbuf(coutbuf);
                std::cout << "\nWould you like to see detailed alternative solutions? (y/n): ";
                char response;
                std::cin >> response;
                
                if (response == 'y' || response == 'Y') {
                    std::cout << "How many alternatives to show? (1-" 
                              << std::min(static_cast<std::size_t>(9), allSolutions.size() - 1) << "): ";
                    int numToShow = 0;
                    std::cin >> numToShow;

                    // Switch back to file for output
                    std::cout.rdbuf(MyFile.rdbuf());

                    // Limit to valid range
                    if (numToShow < 1) numToShow = 1;
                    if (static_cast<std::size_t>(numToShow) > allSolutions.size() - 1) {
                        numToShow = static_cast<int>(allSolutions.size() - 1);
                    }

                    for (int i = 1; i <= numToShow; ++i) {
                        printDetailedSolution(allSolutions[i], i, false);
                    }
                } else {
                    // Switch back to file
                    std::cout.rdbuf(MyFile.rdbuf());
                }
            }
        } else {
            std::cout << "\nNo solution found!" << std::endl;
        }

        // Print statistics about the search
        solver.printStatistics();

    } catch (const std::exception& e) {
        // Make sure we're writing errors to console
        std::cout.rdbuf(coutbuf);
        std::cerr << "Error: " << e.what() << std::endl;
        MyFile.close();
        return 1;
    }

    // Restore original buffer and close file
    std::cout.rdbuf(coutbuf);
    MyFile.close();

    // Show completion message on console
    std::cout << "\n=== PROGRAM COMPLETED ===" << std::endl;
    std::cout << "Results saved to: AStarProcess.txt" << std::endl;
    std::cout << "Best solution moves saved to: BestSolutionMoves.txt" << std::endl;
    
    if (verbose) {
        std::cout << "\nA* debug output was shown above." << std::endl;
        std::cout << "Check AStarProcess.txt for complete results." << std::endl;
    } else {
        std::cout << "\nRun with 'verbose' parameter to see A* debug output on console." << std::endl;
        std::cout << "Example: " << argv[0] << " " << argv[1] << " verbose" << std::endl;
    }

    return 0;
}