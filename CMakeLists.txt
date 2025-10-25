cmake_minimum_required(VERSION 3.10)
project(WtT)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2.5 COMPONENTS graphics audio window system REQUIRED)

add_executable(WtT src/main.cpp)
target_link_libraries(WtT sfml-graphics sfml-audio sfml-window sfml-system)