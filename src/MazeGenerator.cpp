#include "MazeGenerator.h"
#include "Constants.h"
#include <thread>
#include <chrono>
#include <algorithm>

void carveDFS(int startR, int startC, std::vector<std::string>& maze, std::mt19937& gen) {
    if (!isRunning) return;

    int dr[] = {-2, 2, 0, 0}; // { Up, Down, Left, Right }  Move by 2 to make sure walls are created
    int dc[] = {0, 0, -2, 2};
    std::vector<int> dirs = {0, 1, 2, 3};
    std::shuffle(dirs.begin(), dirs.end(), gen); 

    for (int i : dirs) {
        int nr = startR + dr[i];
        int nc = startC + dc[i];

        if (nr > 0 && nr < maze.size() - 1 && nc > 0 && nc < maze[0].size() - 1 && maze[nr][nc] == WALL) {
            {
                std::lock_guard<std::mutex> lock(mazeMutex);
                maze[startR + dr[i] / 2][startC + dc[i] / 2] = EMPTY;
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

struct FrontierNode {
    int r, c, wr, wc; 
};

void carvePrim(int startR, int startC, std::vector<std::string>& maze, std::mt19937& gen) {
    if (!isRunning) return;
    std::vector<FrontierNode> frontiers;
    
    {
        std::lock_guard<std::mutex> lock(mazeMutex);
        maze[startR][startC] = EMPTY;
    }

    int dr[] = {-2, 2, 0, 0};
    int dc[] = {0, 0, -2, 2};

    auto addFrontiers = [&](int r, int c) {
        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i], nc = c + dc[i];
            if (nr > 0 && nr < maze.size() - 1 && nc > 0 && nc < maze[0].size() - 1 && maze[nr][nc] == WALL) {
                frontiers.push_back({nr, nc, r + dr[i] / 2, c + dc[i] / 2});
                std::lock_guard<std::mutex> lock(mazeMutex);
                maze[nr][nc] = FRONTIER; 
            }
        }
    };

    addFrontiers(startR, startC);

    while (!frontiers.empty() && isRunning) {
        std::uniform_int_distribution<> dist(0, frontiers.size() - 1);
        int randomIndex = dist(gen);
        FrontierNode f = frontiers[randomIndex];
        frontiers.erase(frontiers.begin() + randomIndex);

        if (maze[f.r][f.c] == FRONTIER || maze[f.r][f.c] == WALL) {
            {
                std::lock_guard<std::mutex> lock(mazeMutex);
                maze[f.wr][f.wc] = EMPTY; 
                maze[f.r][f.c] = HEAD;    
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