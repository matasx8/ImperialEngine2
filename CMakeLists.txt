cmake_minimum_required(VERSION 3.10)

project(ImperialEngine2 LANGUAGES CXX)

add_library(imperial_engine SHARED Source/Engine.cpp)

set_target_properties(imperial_engine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Include directories for header files
#target_include_directories(MyLibrary PUBLIC
#    ${CMAKE_CURRENT_SOURCE_DIR}/include
#)

set_target_properties(imperial_engine PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${CMAKE_ANDROID_ARCH_ABI}"
)
