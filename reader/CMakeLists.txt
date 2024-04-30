cmake_minimum_required(VERSION 3.0)  # or 3.10, see below

project(drs4reader VERSION 1.0.0)

find_package(EXPAT 2.2.8 MODULE REQUIRED)
find_package(ROOT REQUIRED)

set(INCLUDES ${ROOT_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR})
include_directories(${INCLUDES})

add_executable(drs4_reader
    drs4_reader.cpp
)

# a) for CMake >=3.10 (see CMake's FindEXPAT docs)
target_link_libraries(drs4_reader PUBLIC EXPAT::EXPAT ${ROOT_LIBRARIES})

# b) for CMake >=3.0
#target_include_directories(hello PRIVATE ${EXPAT_INCLUDE_DIRS})
#target_link_libraries(hello PUBLIC ${EXPAT_LIBRARIES})
