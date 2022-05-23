cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -O0")

project(GATSP)

find_package(MPI REQUIRED)

add_executable(GATSP main.cpp
        src/TSPRoute.cpp src/TSPRoute.h
        src/Random.h src/Random.cpp
        src/TravellingSalesman.cpp src/TravellingSalesman.h
        src/MPIController.cpp src/MPIController.h
        src/MPITimer.cpp src/MPITimer.h)

target_link_libraries(GATSP PUBLIC MPI::MPI_CXX)
