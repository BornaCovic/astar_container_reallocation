#ifndef HOTSTORAGESIMULATOR_H
#define HOTSTORAGESIMULATOR_H

#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "Printer.h"

class EntryContainerStack;
class OutGoingContainerStack;
class SingleContainerCrane;
class UntilDue;
class ParsedBuffers;

class HotStorageSimulator {
public:
        explicit HotStorageSimulator(Printer &p);

        void simulate();

        std::vector<std::string> loadBestSolutionMoves();
    std::vector<std::string> recalculateAStar();

private:
        std::mutex recalcMutex;
    std::condition_variable recalcCV;
    std::atomic<bool> needsRecalculation{false};
    std::atomic<bool> systemPaused{false};
    std::mutex movesMutex;

        std::atomic<int> systemTime{0};      std::vector<std::string> currentMoves;

        Printer *printer;

        void runEntryStack();
    void runCrane();
    void runOutgoingStack();

    HotStorageSimulator(const HotStorageSimulator&) = delete;
    HotStorageSimulator& operator=(const HotStorageSimulator&) = delete;
};

#endif 