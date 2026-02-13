#include <iostream>
#include <string>
#include <iomanip>

#include "UntilDueContainer.h"

UntilDue::UntilDue() {}

UntilDue::UntilDue(int minutes, int seconds) : minutes(minutes), seconds(seconds) {}

std::string UntilDue::toString() const {
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds;
    return oss.str();
}

void UntilDue::setMinutes(int minutes){
    this->minutes = minutes;
}

int UntilDue::getMinutes(){
    return minutes;
}

void UntilDue::setSeconds(int seconds){
    this->seconds = seconds;
}

int UntilDue::getSeconds(){
    return seconds;
}

UntilDueContainer::UntilDueContainer() {}

UntilDueContainer::UntilDueContainer(const std::string &id, const UntilDue &untilDue)
    : Container(id), untilDue(untilDue) {}

void UntilDueContainer::displayDetails() const  {
    std::string combinedOutput = id + " " + untilDue.toString();

    std::cout << std::setw(11) << combinedOutput;
}

std::string UntilDueContainer::getDetails() const{
    return id + " " + untilDue.toString();
}

std::string UntilDueContainer::getId() const{return id;}

UntilDue UntilDueContainer::getUntilDue() const { return untilDue; }

void UntilDueContainer::setUntilDue(UntilDue &newUntilDue) { untilDue = newUntilDue; }

// === ADDED METHODS FOR A* SUPPORT ===

void UntilDueContainer::setArrivalTime(int t) { t_arrival = t; }
int UntilDueContainer::getArrivalTime() const { return t_arrival; }

void UntilDueContainer::setDueIn(int seconds) { due_in = seconds; }
int UntilDueContainer::getDueIn() const { return due_in; }

void UntilDueContainer::setExitTime(int t) { exit_time = t; }
int UntilDueContainer::getExitTime() const { return exit_time; }

int UntilDueContainer::getDueTime() const {
    if (t_arrival < 0 || due_in < 0) return -1;
    return t_arrival + due_in;
}

int UntilDueContainer::getLateness() const {
    if (exit_time < 0 || t_arrival < 0 || due_in < 0) return -1;
    int lateness = exit_time - getDueTime();
    return lateness > 0 ? lateness : 0;
}



