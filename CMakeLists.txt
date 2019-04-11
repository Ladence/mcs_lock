cmake_minimum_required(VERSION 3.2)

set(PROJECT "mcs")
set(CMAKE_CXX_STANDARD 14)

find_package(Threads REQUIRED)

add_executable(${PROJECT} mcs_test.cpp)
target_link_libraries(${PROJECT} Threads::Threads)
