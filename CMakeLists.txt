cmake_minimum_required(VERSION 3.20)
project(StudentPlacementInternshipSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(mysql-concpp REQUIRED)

add_executable(StudentPlacementSystem main.cpp)
target_link_libraries(StudentPlacementSystem PRIVATE mysql::concpp-jdbc)
