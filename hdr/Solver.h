#pragma once
#include <vector>
#include <string>

void placeHardestExit(std::vector<std::string>& maze, int startX, int startY, int& outExitX, int& outExitY);
void solveAStar(std::vector<std::string>& maze, int startX, int startY, int exitX, int exitY);