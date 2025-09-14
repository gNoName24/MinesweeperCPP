cmake_minimum_required(VERSION 3.24)
project(MinesweeperCPP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(MinesweeperCPP
    main.cpp

    MinesweeperCPP/MinesweeperCPP_GameGrid.cpp
    MinesweeperCPP/MinesweeperCPP_MinesweeperGame_Save.cpp
    MinesweeperCPP/MinesweeperCPP_MinesweeperGame_Run.cpp
)
