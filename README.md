# Container Reallocation Optimization using A*

This project implements an A* based solver for the container relocation problem inspired by automated port storage systems written in C++.

The system models a simplified container yard with one entry stack, three buffer stacks, one outgoing stack, and a single crane that can move containers between them. Each container has an arrival time and a due time, and the objective is to minimize the total lateness of all containers leaving the system.

The main challenge comes from LIFO stack constraints, limited buffer capacity, discrete departure times, and the fact that the crane can only hold one container at a time.

## Approach

The solution uses the A* search algorithm with:

- Explicit state space representation including crane state
- A cost function that penalizes lateness, idle time, and remaining containers
- An admissible heuristic that estimates minimal possible future lateness
- State hashing to avoid revisiting identical configurations
- Priority queue ordered by f(n) = g(n) + h(n)

The implementation guarantees optimal solutions as long as the heuristic remains admissible.

## Evaluation

The system was tested on instances with 3, 5, and 8 containers.

The results show consistent optimal behaviour for smaller instances, while also highlighting the expected state space growth as the number of containers increases.

## Notes

This project was developed as part of my bachelor thesis. It focuses on correctness and modelling accuracy rather than large scale industrial performance.
