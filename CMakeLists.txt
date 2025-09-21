cmake_minimum_required(VERSION 3.24)
project(MinesweeperCPP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(MinesweeperCPP
    main.cpp

    MinesweeperCPP/MinesweeperCPP.cpp
    MinesweeperCPP/Keyboard.cpp
    MinesweeperCPP/GameGrid.cpp
    MinesweeperCPP/Scenes.cpp
    MinesweeperCPP/Notifications.cpp
    MinesweeperCPP/MinesweeperGame/Save.cpp
    MinesweeperCPP/MinesweeperGame/Run.cpp

    MinesweeperCPP/MinesweeperGame/Default/Default.cpp
    MinesweeperCPP/MinesweeperGame/History/History.cpp
)
include(FetchContent)

FetchContent_Declare(
    InitTools
    GIT_REPOSITORY https://github.com/gNoName24/InitTools.git
    GIT_TAG        v2.0.0
)
FetchContent_MakeAvailable(InitTools)

target_link_libraries(MinesweeperCPP PUBLIC InitConsole)
