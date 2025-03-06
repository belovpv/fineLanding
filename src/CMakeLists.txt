
cmake_minimum_required(VERSION 3.13.5)
project(mavlink1 VERSION 0.1.0 LANGUAGES C CXX)

cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0015 NEW)
cmake_policy(SET CMP0076 NEW)


find_package(nlohmann_json 3.11.3 REQUIRED)
find_library(libconfig libconfig)

add_executable(finelanding main.cpp)

add_subdirectory(main)

target_link_libraries(finelanding PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(finelanding PRIVATE config++)

