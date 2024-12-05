cmake_minimum_required(VERSION 3.29)
project(Midterm-Assignment_AoS)

set(CMAKE_CXX_STANDARD 20)

# Comment this on Windows
set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")

# For Mac
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xpreprocessor -fopenmp -lomp -fuse-ld=lld")

# For Windows
# set(CMAKE_C_COMPILER "/opt/local/bin/clang++-mp-3.9")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp")

# Aggiungi ottimizzazioni solo in modalità Release
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# Comment this on Windows
include_directories("/usr/local/opt/libomp/include")
link_directories("/usr/local/opt/libomp/lib")

add_executable(Midterm-Assignment_AoS main.cpp
        findeBestMatch.cpp
        findeBestMatch.h
        utils.cpp
        utils.h
        timeSeries.h)