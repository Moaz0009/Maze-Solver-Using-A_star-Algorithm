cmake_minimum_required(VERSION 3.20)
project(MazeSolver)

set(CMAKE_CXX_STANDARD 17)

# --- THE FIX: FORCE STATIC LINKING ---
# 1. Tell SFML to bundle itself inside the .exe (No sfml-*.dll files needed)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force Static SFML" FORCE)

# 2. Tell MinGW to bundle the standard C++ libraries (No libstdc++-6.dll needed)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
# -------------------------------------

# --- AUTOMATIC SFML DOWNLOADER ---
include(FetchContent)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1 
)

FetchContent_MakeAvailable(SFML)
# ---------------------------------

add_executable(MazeSolver
        src/main.cpp
        src/MazeGenerator.cpp
        src/Solver.cpp
)
target_include_directories(MazeSolver PRIVATE hdr)
# Attach the downloaded SFML libraries to your code
target_link_libraries(MazeSolver PRIVATE sfml-graphics sfml-window sfml-system)

