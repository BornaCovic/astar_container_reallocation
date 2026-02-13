#ifndef UNTILDUECONTAINER_H
#define UNTILDUECONTAINER_H

#include "Container.h"
#include <string>

class UntilDue {
    public:
        UntilDue();
        UntilDue(int minutes, int seconds);
        void setMinutes(int minutes);
        int getMinutes();
        void setSeconds(int seconds);
        int getSeconds();
        std::string toString() const;
    private:
        int minutes;
        int seconds;
};

class UntilDueContainer : public Container {
    public:
        UntilDueContainer();
        UntilDueContainer(const std::string &id, const UntilDue &untilDue);
        void displayDetails() const;
        std::string getDetails() const;
        UntilDue getUntilDue() const;
        void setUntilDue(UntilDue &newUntilDue);
        // ======== A* SUPPORT METHODS (ADDED) =========
        void setArrivalTime(int t);
        int getArrivalTime() const;

        std::string getId() const;

        void setDueIn(int seconds);
        int getDueIn() const;

        void setExitTime(int t);
        int getExitTime() const;

        int getDueTime() const;      // t_arrival + due_in
        int getLateness() const;     // max(0, exit_time - due_time)
        // =============================================
    private:
        UntilDue untilDue;
        int t_arrival = -1;   // Time when container entered the system
        int due_in = -1;      // Allowed time to move to exit (in seconds)
        int exit_time = -1;   // Time it reached the exit stack
};

#endif //UNTILDUECONTAINER_H