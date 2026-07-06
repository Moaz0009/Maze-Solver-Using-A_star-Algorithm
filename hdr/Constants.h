
#pragma once
#include <mutex>
#include <atomic>

// C++17 allows us to define constants right in the header cleanly
constexpr char WALL = '#';
constexpr char EMPTY = '.';
constexpr char START = 'S';
constexpr char EXIT = 'E';
constexpr char HEAD = 'H';
constexpr char FRONTIER = 'F';

// Tell all files that these variables exist somewhere (they are defined in main.cpp)
extern std::mutex mazeMutex;
extern std::atomic<bool> isRunning;
