cmake_minimum_required(VERSION 3.10)
project(WtT)

set(CMAKE_CXX_STANDARD 17)
#set(SFML_STATIC_LIBRARIES TRUE)

# Directorios
set(SRC_DIR "${CMAKE_SOURCE_DIR}/assets")
set(DEST_DIR "${CMAKE_BINARY_DIR}/assets")

set(SFML_DIR "C:/SFML/lib/cmake/SFML")

#find_package(SFML 2.6 COMPONENTS graphics audio window system REQUIRED)

add_executable(WtT src/main.cpp)
target_link_libraries(WtT sfml-graphics sfml-audio sfml-window sfml-system)