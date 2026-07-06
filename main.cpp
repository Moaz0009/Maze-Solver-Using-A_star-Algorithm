#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <random> // Included for modern C++ random number generation

// Define our maze characters
const char WALL = '#';
const char EMPTY = '.';
const char START = 'S';
const char EXIT = 'E';

// The Node represents a single coordinate in our maze
struct Node {
    int x, y;
    int gCost, hCost, fCost;
    Node* parent;
    Node(int x, int y) : x(x), y(y), gCost(0), hCost(0), fCost(0), parent(nullptr) {}
};

// Function to generate a random maze
std::vector<std::string> generateRandomMaze(int rows, int cols, int wallDensityPercent) {
    // Initialize a blank maze filled with empty spaces
    std::vector<std::string> maze(rows, std::string(cols, EMPTY));

    // Setup the random number generator
    std::random_device rd;  // Obtain a random number from hardware
    std::mt19937 gen(rd()); // Seed the generator
    std::uniform_int_distribution<> distrib(1, 100); // Define the range [1, 100]

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            // Create the solid outer boundary walls
            if (r == 0 || r == rows - 1 || c == 0 || c == cols - 1) {
                maze[r][c] = WALL;
            }
            // Randomly place internal walls based on the density percentage
            else if (distrib(gen) <= wallDensityPercent) {
                maze[r][c] = WALL;
            }
        }
    }

    // Hardcode the Start (top-left) and Exit (bottom-right)
    maze[1][1] = START;
    maze[rows - 2][cols - 2] = EXIT;

    // Clear immediate adjacent spaces to prevent the Start/Exit from being instantly boxed in
    if (cols > 3 && rows > 3) {
        maze[1][2] = EMPTY;
        maze[2][1] = EMPTY;
        maze[rows - 2][cols - 3] = EMPTY;
        maze[rows - 3][cols - 2] = EMPTY;
    }

    return maze;
}

// Function to print the current state of the maze
void printMaze(const std::vector<std::string>& maze) {
    std::cout << "\033[2J\033[1;1H\n"; // Clear screen

    for (const auto& row : maze) {
        std::cout << row << "\n";
    }
    std::cout << "----------------------\n";
}

int main() {
    // Generate a maze: 30 rows, 90 columns, with a 25% chance of walls
    std::vector<std::string> myMaze = generateRandomMaze(30, 90, 25);

    printMaze(myMaze);
    
    return 0;
}