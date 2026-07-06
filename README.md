# Maze Solver Using A* Algorithm

A professional-grade Maze Generation and Pathfinding visualizer built in C++ and SFML.

## Features
* **Maze Generation:** Supports both **Recursive Backtracking (DFS)** for long, winding corridors and **Randomized Prim's Algorithm** for complex, branching structures.
* **Intelligent Pathfinding:** Solves the generated maze using the **A* Search Algorithm**, prioritizing the shortest path using a Manhattan distance heuristic.
* **Topology Analysis:** Automatically calculates the mathematically hardest exit point using a **Breadth-First Search (BFS)** flood-fill algorithm.
* **High-Performance Visualization:** Uses **SFML** for smooth, multi-threaded rendering, ensuring the algorithm logic never blocks the UI.

## Modular Structure
* `/hdr`: Header files containing definitions and shared constants.
* `/src`: Source files containing the generation, solving, and UI logic.
* `/CMakeLists.txt`: Build configuration using static linking for portability.

## Prerequisites
* **SFML 2.6.1+**
* **CMake 3.20+**
* **MinGW (for Windows)**

## Building the Project
This project uses CMake. To build it, run the following in your terminal:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```
## Running the Project

Once the build is complete, the executable will be generated inside the `build/Debug` directory. 
You can run it directly from the terminal:

```bash
./Debug/MazeSolver.exe
```

## How to Use

In `main.cpp`, you can select your preferred generation algorithm:
* Set `int mazeAlgorithmChoice = 1;` for DFS (Backtracking).
* Set `int mazeAlgorithmChoice = 2;` for Prim's Algorithm.
