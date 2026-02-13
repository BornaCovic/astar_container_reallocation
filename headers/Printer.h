#ifndef PRINTER_H
#define PRINTER_H
#include <iostream>
#include <iomanip>

#include "ParsedBuffers.h"
#include "SingleContainerCrane.h"
#include <string>

class Printer {
private:
    ParsedBuffers* pb;
    Crane* crane;
    int NArrivals;
    int NBuffers;
    int NHoldovers;
    int getNumberOfStacksInSameCategory(char c);
    int calculateIndexOfHook();
    int calculateHeight();
    std::string centerText(const std::string& text, int width);
    bool sameCategoryStack(const std::string &stack1,const std::string &stack2);
public:
    Printer(ParsedBuffers &pb , Crane &crane);
    ~Printer();
    void printEverything();
    ParsedBuffers *getParsedBuffers();
    Crane *getCrane();
};
#endif