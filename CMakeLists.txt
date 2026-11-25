cmake_minimum_required(VERSION 3.20)
project(UavMoeaArchiving VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(nlohmann_json)

set(SOURCES
    src/core/main.cpp
    src/core/Algorithm.cpp
    src/core/AsyncLogger.cpp
    src/core/Experiments.cpp

    src/archivers/Nsga2Archiver.cpp
    src/archivers/IndicatorBasedArchiver.cpp
    src/archivers/MultiLevelGridArchiver.cpp

    src/problems/Zdt1Problem.cpp
    src/problems/UavPathProblem.cpp
    src/problems/UavInstance.cpp

    src/selections/RandomSelection.cpp
    
    src/variations/SbxVariation.cpp
    src/variations/UavVariation.cpp

    src/indicators/R2Indicator.cpp
    src/indicators/IgdPlusIndicator.cpp
    src/indicators/EpsilonIndicator.cpp

    src/utils/Pareto.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
get_target_property(JSON_INC nlohmann_json::nlohmann_json INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${JSON_INC})
target_compile_definitions(${PROJECT_NAME} PRIVATE UAV_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()