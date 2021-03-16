cmake_minimum_required(VERSION 3.5)

project(BKTurbo8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_PATH ${CMAKE_HOME_DIRECTORY}/Lib)

file(GLOB BKbin2obj_SRC
     "BKbin2obj.h"
     "BKbin2obj.cpp"
)

add_executable(BKbin2obj ${BKbin2obj_SRC} ${BKTurbo8Lib_SRC})
target_link_libraries(BKbin2obj LINK_PUBLIC BKTurbo8Lib)
