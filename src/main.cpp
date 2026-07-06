#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <random>

// Include our custom modular files
#include "Constants.h"
#include "MazeGenerator.h"
#include "Solver.h"

// Define the global thread variables declared in Constants.h
std::mutex mazeMutex;
std::atomic<bool> isRunning{true};

void runPipeline(std::vector<std::string>& maze, int startX, int startY, int algoChoice) {
    std::random_device rd;
    std::mt19937 gen(rd());
    int exitX = 0, exitY = 0;

    std::cout << "[System] Carving maze...\n";
    auto genStart = std::chrono::high_resolution_clock::now();

    if (algoChoice == 1) {
        { std::lock_guard<std::mutex> lock(mazeMutex); maze[startX][startY] = EMPTY; }
        carveDFS(startX, startY, maze, gen);
    } else {
        carvePrim(startX, startY, maze, gen);
        std::lock_guard<std::mutex> lock(mazeMutex);
        for(auto& row : maze) {
            for(char& c : row) if(c == FRONTIER) c = WALL;
        }
    }

    auto genEnd = std::chrono::high_resolution_clock::now();
    auto genTime = std::chrono::duration_cast<std::chrono::milliseconds>(genEnd - genStart).count();
    std::cout << "[System] Maze generated in " << genTime << " ms.\n";

    placeHardestExit(maze, startX, startY, exitX, exitY);

    {
        std::lock_guard<std::mutex> lock(mazeMutex);
        maze[startX][startY] = START;
        maze[exitX][exitY] = EXIT;
    }

    std::cout << "[System] Hardest exit placed. Starting A* Solver...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    solveAStar(maze, startX, startY, exitX, exitY);
}

int main() {
    int mazeAlgorithmChoice = 2; // 1 = DFS, 2 = Prim's

    int rows = 31;
    int cols = 51;
    int cellSize = 18;

    std::vector<std::string> myMaze(rows, std::string(cols, WALL));
    int startX = 1, startY = 1;

    sf::RenderWindow window(sf::VideoMode(cols * cellSize, rows * cellSize), "A* Maze Solver");
    window.setFramerateLimit(60);

    std::thread pipelineThread(runPipeline, std::ref(myMaze), startX, startY, mazeAlgorithmChoice);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                isRunning = false;
                window.close();
            }
        }

        window.clear(sf::Color::Black);

        std::lock_guard<std::mutex> lock(mazeMutex);
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                sf::RectangleShape cell(sf::Vector2f(cellSize - 1, cellSize - 1));
                cell.setPosition(c * cellSize, r * cellSize);

                char tile = myMaze[r][c];
                if      (tile == WALL)     cell.setFillColor(sf::Color(30, 30, 45));
                else if (tile == EMPTY)    cell.   setFillColor(sf::Color(240, 240, 240));
                else if (tile == HEAD)     cell.setFillColor(sf::Color(255, 100, 200));
                else if (tile == FRONTIER) cell.setFillColor(sf::Color(255, 165, 0));
                else if (tile == START)    cell.setFillColor(sf::Color(50, 200, 50));
                else if (tile == EXIT)     cell.setFillColor(sf::Color(200, 50, 50));
                else if (tile == 'O')      cell.setFillColor(sf::Color(255, 200, 0));
                else if (tile == 'x')      cell.setFillColor(sf::Color(150, 200, 255));
                else if (tile == '*')      cell.setFillColor(sf::Color(200, 0, 255));

                window.draw(cell);
            }
        }
        window.display();
    }

    pipelineThread.join();
    return 0;
}