
#pragma once
#include <mutex>
#include <atomic>


constexpr char WALL = '#';
constexpr char EMPTY = '.';
constexpr char START = 'S';
constexpr char EXIT = 'E';
constexpr char HEAD = 'H';
constexpr char FRONTIER = 'F';


extern std::mutex mazeMutex;
extern std::atomic<bool> isRunning;
