cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(Halcyon VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
#include_directories(${CMAKE_SOURCE_DIR})
add_executable(Halcyon src/main.cpp src/tokenizer.cpp)

# Optionally, specify include directories or libraries if needed
# include_directories(${PROJECT_SOURCE_DIR}/include)
# target_link_libraries(HelloWorld some_library)