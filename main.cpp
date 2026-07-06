#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>
#include <atomic>
#include <cmath>
#include <queue>
#include <algorithm>

const char WALL = '#';
const char EMPTY = '.';
const char START = 'S';
const char EXIT = 'E';
const char HEAD = 'H';     // Used for DFS head
const char FRONTIER = 'F'; // Used for Prim's expanding frontier

std::mutex mazeMutex;
std::atomic<bool> isRunning(true);

// --- A* STRUCTURES ---
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


// --- PHASE 1A: DFS MAZE GENERATION (The Long Rivers) ---
void carveDFS(int r, int c, std::vector<std::string>& maze, std::mt19937& gen) {
    if (!isRunning) return;

    int dr[] = {-2, 2, 0, 0};
    int dc[] = {0, 0, -2, 2};
    std::vector<int> dirs = {0, 1, 2, 3};
    std::shuffle(dirs.begin(), dirs.end(), gen);

    for (int i : dirs) {
        int nr = r + dr[i];
        int nc = c + dc[i];

        if (nr > 0 && nr < maze.size() - 1 && nc > 0 && nc < maze[0].size() - 1 && maze[nr][nc] == WALL) {
            {
                std::lock_guard<std::mutex> lock(mazeMutex);
                maze[r + dr[i] / 2][c + dc[i] / 2] = EMPTY;
                maze[nr][nc] = HEAD;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(15));

            {
                std::lock_guard<std::mutex> lock(mazeMutex);
                maze[nr][nc] = EMPTY;
            }
            carveDFS(nr, nc, maze, gen);
        }
    }
}

// --- PHASE 1B: PRIM'S MAZE GENERATION (The Shattered Branches) ---
struct FrontierNode {
    int r, c;   // The target cell to carve
    int wr, wc; // The wall between the maze and the target cell
};

void carvePrim(int startR, int startC, std::vector<std::string>& maze, std::mt19937& gen) {
    std::vector<FrontierNode> frontiers;

    // Start by carving the first cell
    {
        std::lock_guard<std::mutex> lock(mazeMutex);
        maze[startR][startC] = EMPTY;
    }

    int dr[] = {-2, 2, 0, 0};
    int dc[] = {0, 0, -2, 2};

    // Helper lambda to add new frontiers
    auto addFrontiers = [&](int r, int c) {
        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i], nc = c + dc[i];
            if (nr > 0 && nr < maze.size() - 1 && nc > 0 && nc < maze[0].size() - 1 && maze[nr][nc] == WALL) {
                frontiers.push_back({nr, nc, r + dr[i] / 2, c + dc[i] / 2});
                std::lock_guard<std::mutex> lock(mazeMutex);
                maze[nr][nc] = FRONTIER; // Mark visually so we can see the algorithm "thinking"
            }
        }
    };

    addFrontiers(startR, startC);

    while (!frontiers.empty() && isRunning) {
        // Pick a completely random frontier
        std::uniform_int_distribution<> dist(0, frontiers.size() - 1);
        int randomIndex = dist(gen);
        FrontierNode f = frontiers[randomIndex];

        // Remove it from the list
        frontiers.erase(frontiers.begin() + randomIndex);

        if (maze[f.r][f.c] == FRONTIER || maze[f.r][f.c] == WALL) {
            {
                std::lock_guard<std::mutex> lock(mazeMutex);
                maze[f.wr][f.wc] = EMPTY; // Smash the wall
                maze[f.r][f.c] = HEAD;    // Mark head briefly for animation
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(15));

            {
                std::lock_guard<std::mutex> lock(mazeMutex);
                maze[f.r][f.c] = EMPTY;
            }

            addFrontiers(f.r, f.c);
        }
    }
}

// --- PHASE 2: BFS TRUE HARDEST EXIT FINDER ---
// This runs instantly in the background to find the topologically furthest point
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

    // Standard BFS floods the maze. The very last cell it checks is the furthest away.
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

    // Pass the furthest coordinates back out
    outExitX = furthestX;
    outExitY = furthestY;
}


// --- PHASE 3: A* SOLVER (With Statistics) ---
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

    // 1. STAT TRACKING: Start the solver clock and initialize counters
    int nodesEvaluated = 0;
    auto solveStart = std::chrono::high_resolution_clock::now();

    while (!openSet.empty() && isRunning) {
        Node* current = openSet.top();
        openSet.pop();

        nodesEvaluated++; // Increment every time we pull a new box to evaluate

        if (current->x == exitX && current->y == exitY) {
            // 2. STAT TRACKING: Stop the solver clock
            auto solveEnd = std::chrono::high_resolution_clock::now();
            auto solveTime = std::chrono::duration_cast<std::chrono::milliseconds>(solveEnd - solveStart).count();

            int pathLength = 0;
            Node* pathNode = current->parent;
            while (pathNode != nullptr && pathNode->parent != nullptr) {
                std::lock_guard<std::mutex> lock(mazeMutex);
                maze[pathNode->x][pathNode->y] = '*';
                pathNode = pathNode->parent;
                pathLength++; // Increment for every box in the final path
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }

            // 3. STAT PRINTING: Output the final summary to the CLion Console
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


// --- THE PIPELINE THREAD (With Generation Stats) ---
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
    // ==========================================
    // OPTIONS: Change this to test the algorithms!
    // 1 = Recursive Backtracking (DFS)
    // 2 = Prim's Algorithm
    int mazeAlgorithmChoice = 2;
    // ==========================================

    int rows = 31; // Slightly larger odd numbers make Prim's look better
    int cols = 51;
    int cellSize = 18;

    std::vector<std::string> myMaze(rows, std::string(cols, WALL));
    int startX = 1, startY = 1;

    sf::RenderWindow window(sf::VideoMode(cols * cellSize, rows * cellSize), "A* Maze Solver - Dynamic Exit");
    window.setFramerateLimit(60);

    // Launch the pipeline and pass our algorithm choice
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
                else if (tile == EMPTY)    cell.setFillColor(sf::Color(240, 240, 240));
                else if (tile == HEAD)     cell.setFillColor(sf::Color(255, 100, 200));
                else if (tile == FRONTIER) cell.setFillColor(sf::Color(255, 165, 0));   // Orange for Prim's frontiers
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