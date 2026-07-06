#pragma once
#include <vector>
#include <string>
#include <random>

void carveDFS(int startR, int startC, std::vector<std::string>& maze, std::mt19937& gen);
void carvePrim(int startR, int startC, std::vector<std::string>& maze, std::mt19937& gen);