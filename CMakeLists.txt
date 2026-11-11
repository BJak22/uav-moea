cmake_minimum_required(VERSION 3.20)
project(UavMoeaArchiving VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    src/core/main.cpp
    src/core/Algorithm.cpp
    src/core/AsyncLogger.cpp

    src/archivers/Nsga2Archiver.cpp

    src/problems/Zdt1Problem.cpp

    src/selections/RandomSelection.cpp
    
    src/variations/SbxVariation.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()