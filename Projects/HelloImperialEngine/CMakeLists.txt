cmake_minimum_required(VERSION 3.10)

project(HelloImperialEngine LANGUAGES CXX)

add_library(application SHARED main.cpp)

set_target_properties(application PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

set_target_properties(application PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${CMAKE_ANDROID_ARCH_ABI}"
)

# Extra step to add the application lib to the Android Studio project
set(SECONDARY_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/../../AndroidStudio/app/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}")
add_custom_command(TARGET application POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${CMAKE_ANDROID_ARCH_ABI}/libapplication.so"
        "${SECONDARY_OUTPUT_DIR}/libapplication.so"
    COMMENT "Copying libapplication.so to ${SECONDARY_OUTPUT_DIR}"
)

target_include_directories(application PUBLIC "../../Source")
target_link_libraries(application PUBLIC "${CMAKE_BINARY_DIR}/../../../build/${CMAKE_BUILD_TYPE}/${CMAKE_ANDROID_ARCH_ABI}/libimperial_engine.so")

find_library(log-lib log)
target_link_libraries(application PUBLIC ${log-lib})