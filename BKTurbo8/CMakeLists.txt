cmake_minimum_required(VERSION 3.5)

project(BKTurbo8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_PATH ${CMAKE_HOME_DIRECTORY}"/Lib")

file(GLOB BKTurbo8_SRC
     "BKTurbo8.h"
     "BKTurbo8.cpp"

)

add_executable(BKTurbo8 ${BKTurbo8_SRC})
target_link_libraries(BKTurbo8 LINK_PUBLIC BKTurbo8Lib)

