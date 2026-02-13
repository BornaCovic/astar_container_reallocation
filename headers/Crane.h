#ifndef CRANE_H
#define CRANE_H

#include <string>
#include "ParsedBuffers.h"
#include <vector>

class Crane
{
protected:
    std::string name;
    std::string aboveStack;
    std::vector<std::string> stackNames;
    int aboveStackIndex;
    UntilDueContainer heldContainer;
    bool hasHeldContainer = false;

public:
    Crane(const std::string &name, const std::vector<std::string> &stackNames);

    virtual ~Crane() = default;

    std::string getName() const;
    void setName(const std::string &newName);

    std::string getAboveStack() const;
    void setAboveStack(std::string newAboveStack);

    int getAboveStackIndex() const;
    void setAboveStackIndex(int index);

    // Apstraktne metode za sadržaj kuke
    virtual void *getHookContent() const = 0;
    virtual void setHookContent(void *newContents) = 0;

    void setHeldContainer(const UntilDueContainer& c);
    void clearHeldContainer();
    const UntilDueContainer* getHeldContainer() const;

};

#endif