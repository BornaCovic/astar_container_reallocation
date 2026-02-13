#ifndef ASTAR_STARTING_STATE_H
#define ASTAR_STARTING_STATE_H

#include "AStarState.h"
#include "ParsedBuffers.h"
#include "SingleContainerCrane.h"

AStarState makeAStarInitialState(ParsedBuffers& parsedBuffers);

AStarState makeAStarCurrentState(ParsedBuffers& parsedBuffers, int currentSystemTime, SingleContainerCrane* crane);

void updateContainerDueTimes(AStarState& state, int elapsedTime);

void printInitialStateInfo(const AStarState& state);
void printCurrentStateInfo(const AStarState& state, int systemTime);

#endif 