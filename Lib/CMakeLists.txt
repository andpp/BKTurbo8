cmake_minimum_required(VERSION 3.5)

file(GLOB BKTurbo8Lib_SRC
     ${CMAKE_HOME_DIRECTORY}/Lib/*.h
     ${CMAKE_HOME_DIRECTORY}/Lib/Assemble.cpp
     ${CMAKE_HOME_DIRECTORY}/Lib/Globals.cpp
     ${CMAKE_HOME_DIRECTORY}/Lib/LabelTable.cpp
     ${CMAKE_HOME_DIRECTORY}/Lib/Object.cpp
     ${CMAKE_HOME_DIRECTORY}/Lib/BKToken.cpp
     ${CMAKE_HOME_DIRECTORY}/Lib/ErrorManager.cpp
     ${CMAKE_HOME_DIRECTORY}/Lib/LabelManager.cpp
     ${CMAKE_HOME_DIRECTORY}/Lib/Listing.cpp
     ${CMAKE_HOME_DIRECTORY}/Lib/Parser.cpp
     ${CMAKE_HOME_DIRECTORY}/Lib/Reader.cpp
     ${CMAKE_HOME_DIRECTORY}/Lib/pch.cpp
) 

add_library (BKTurbo8Lib STATIC ${BKTurbo8Lib_SRC})
target_include_directories (BKTurbo8Lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
