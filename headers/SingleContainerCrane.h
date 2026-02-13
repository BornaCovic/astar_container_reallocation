#ifndef SINGLECONTAINERCRANE_H
#define SINGLECONTAINERCRANE_H

#include "Crane.h"

class SingleContainerCrane : public Crane
{
private:
    UntilDueContainer *container; // Pokazivač na kontejner

public:
    // Konstruktor samo s imenom
    SingleContainerCrane(const std::string &name, const std::vector<std::string> &stackNames)
        : Crane(name, stackNames) {}

    // Implementacija apstraktnih metoda
    void *getHookContent() const override
    {
        return this->container;
    }

    void setHookContent(void *newContents) override
    {
        this->container = static_cast<UntilDueContainer*>(newContents);
    }

    void refreshTime(UntilDue time);

};

#endif // SINGLECONTAINERCRANE_H
