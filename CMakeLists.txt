cmake_minimum_required(VERSION 3.20)

set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

project(bsp)

add_compile_definitions(APP_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(
    ${CMAKE_PROJECT_NAME}
    src/main.cpp
)

target_include_directories(
    ${CMAKE_PROJECT_NAME}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"   
)
target_include_directories(
    ${CMAKE_PROJECT_NAME}
    PRIVATE "${LIB_DIR}/assimp/include"
)

target_link_directories(
    ${CMAKE_PROJECT_NAME}
    PRIVATE "${LIB_DIR}/assimp/lib"
)

target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    PRIVATE assimp
)
if (LINUX)
    target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        PRIVATE m
        PRIVATE stdc++
    )
endif(LINUX)