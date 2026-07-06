#include "Solver.h"
#include "Constants.h"
#include <queue>
#include <cmath>
#include <thread>
#include <chrono>
#include <iostream>

// The Node struct is only needed inside this specific file, so we hide it here
struct Node {
    int x, y, gCost, hCost, fCost;
    Node* parent;
    Node(int x, int y) : x(x), y(y), gCost(0), hCost(0), fCost(0), parent(nullptr) {}
};

int getHeuristic(int x, int y, int exitX, int exitY) {
    return std::abs(x - exitX) + std::abs(y - exitY);
}

struct CompareNode {
    bool operator()(Node* a, Node* b) { return a->fCost > b->fCost; }
};

void placeHardestExit(std::vector<std::string>& maze, int startX, int startY, int& outExitX, int& outExitY) {
    int rows = maze.size();
    int cols = maze[0].size();
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::queue<std::pair<int, int>> q;

    q.push({startX, startY});
    visited[startX][startY] = true;

    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};

    int furthestX = startX;
    int furthestY = startY;

    while (!q.empty()) {
        auto current = q.front();
        q.pop();

        furthestX = current.first;
        furthestY = current.second;

        for (int i = 0; i < 4; i++) {
            int nx = furthestX + dx[i];
            int ny = furthestY + dy[i];

            if (nx >= 0 && nx < rows && ny >= 0 && ny < cols &&
                maze[nx][ny] == EMPTY && !visited[nx][ny]) {
                visited[nx][ny] = true;
                q.push({nx, ny});
            }
        }
    }

    outExitX = furthestX;
    outExitY = furthestY;
}

void solveAStar(std::vector<std::string>& maze, int startX, int startY, int exitX, int exitY) {
    if (!isRunning) return;

    int rows = maze.size();
    int cols = maze[0].size();
    std::priority_queue<Node*, std::vector<Node*>, CompareNode> openSet;
    std::vector<std::vector<bool>> closedSet(rows, std::vector<bool>(cols, false));

    Node* startNode = new Node(startX, startY);
    startNode->hCost = getHeuristic(startX, startY, exitX, exitY);
    startNode->fCost = startNode->hCost;
    openSet.push(startNode);
    closedSet[startX][startY] = true;

    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};

    int nodesEvaluated = 0;
    auto solveStart = std::chrono::high_resolution_clock::now();

    while (!openSet.empty() && isRunning) {
        Node* current = openSet.top();
        openSet.pop();

        nodesEvaluated++;

        if (current->x == exitX && current->y == exitY) {
            auto solveEnd = std::chrono::high_resolution_clock::now();
            auto solveTime = std::chrono::duration_cast<std::chrono::milliseconds>(solveEnd - solveStart).count();

            int pathLength = 0;
            Node* pathNode = current->parent;
            while (pathNode != nullptr && pathNode->parent != nullptr) {
                std::lock_guard<std::mutex> lock(mazeMutex);
                maze[pathNode->x][pathNode->y] = '*';
                pathNode = pathNode->parent;
                pathLength++;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }

            std::cout << "\n====================================\n";
            std::cout << "          MAZE SOLVED!              \n";
            std::cout << "====================================\n";
            std::cout << " Solve Time:       " << solveTime << " ms\n";
            std::cout << " Path Length:      " << pathLength + 1 << " boxes\n";
            std::cout << " Boxes Searched:   " << nodesEvaluated << " boxes\n";
            std::cout << " Efficiency Ratio: " << (pathLength * 100) / (nodesEvaluated == 0 ? 1 : nodesEvaluated) << "%\n";
            std::cout << "====================================\n\n";
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mazeMutex);
            if (maze[current->x][current->y] != START) maze[current->x][current->y] = 'x';
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(15));

        for (int i = 0; i < 4; i++) {
            int newX = current->x + dx[i];
            int newY = current->y + dy[i];

            if (newX >= 0 && newX < rows && newY >= 0 && newY < cols &&
                maze[newX][newY] != WALL && !closedSet[newX][newY]) {

                Node* neighbor = new Node(newX, newY);
                neighbor->gCost = current->gCost + 1;
                neighbor->hCost = getHeuristic(newX, newY, exitX, exitY);
                neighbor->fCost = neighbor->gCost + neighbor->hCost;
                neighbor->parent = current;

                {
                    std::lock_guard<std::mutex> lock(mazeMutex);
                    if (maze[newX][newY] != EXIT) maze[newX][newY] = 'O';
                }

                openSet.push(neighbor);
                closedSet[newX][newY] = true;
            }
        }
    }
}