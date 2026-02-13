#include "SingleContainerCrane.h"

void SingleContainerCrane::refreshTime(UntilDue refresh){
    int refreshSeconds = refresh.getSeconds();
    int refreshMinutes = refresh.getMinutes();

    UntilDueContainer *udc = container;
            
    int min = udc->getUntilDue().getMinutes()-refreshMinutes, sec = udc->getUntilDue().getSeconds() - refreshSeconds;

    if(min > 0 && sec < 0 || min <= 0 && sec <= -60){
        min -= 1;
        sec += 60;
    }

    UntilDue time(min, sec);
    udc->setUntilDue(time);
}