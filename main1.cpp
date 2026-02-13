#include <iostream>
#include "HotStorageSimulator.h"
#include "ParsedBuffers.h"
#include "SingleContainerCrane.h"
#include "Printer.h"

int main(int argc, char* argv[]) {
    // Use command line argument for config file, or default to "ulaz.txt"
    std::string configFile = (argc > 1) ? argv[1] : "ulaz.txt";
    
    ParsedBuffers *data = new ParsedBuffers(configFile);
    Crane *crane = new SingleContainerCrane("CRANE", data->getStackNames());
    Printer *printer = new Printer(*data, *crane);
    HotStorageSimulator simulator(*printer);
    simulator.simulate();
    
    // Clean up
    delete printer;
    delete crane;
    delete data;
    
    return 0;
}